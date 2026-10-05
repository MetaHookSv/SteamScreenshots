# AGENTS.md - SteamScreenshots Project Guide

## Project Overview

**SteamScreenshots** is a MetaHookSV plugin that takes over the GoldSrc / SvEngine `snapshot` console command. Instead of the engine's own screenshot path it captures the current OpenGL framebuffer just before the frame leaves the back buffer and submits the image to the Steam screenshot manager through Steamworks `ISteamScreenshots` — reached via the shared `SteamAPIBridge.dll`, never through a direct `steam_api.dll` import.

- **Project type**: Native C++ plugin (Windows DLL), MSVC x86 only
- **Engine**: GoldSrc / SvEngine (see Engine Compatibility)
- **Framework**: MetaHookSV Plugin API (`IPluginsV4`); the gamedata path needs MetaHook API 109, checked at runtime rather than by a `static_assert`
- **Main dependencies**: MetaHook SDK, SteamAPIBridge (shared DLL), SteamSDK headers (read-only), GLEW (configured and built as a static library), `opengl32`, Capstone headers (used through the MetaHook API, **never linked** — `CAPSTONE_LIBRARY_DIRS` is explicitly ignored). GLFW is test-only

## Project Structure

```
SteamScreenshots/
├── src/
│   ├── plugins.cpp            # IPluginsV4 lifecycle: Init / LoadEngine / LoadClient / ExitGame
│   ├── plugins.h              # Shared globals, MHPluginName, private_funcs_t, legacy search macros
│   ├── exportfuncs.cpp        # Presentation hooks, Steam callbacks, ServerName message, HUD bodies
│   ├── exportfuncs.h          # Hook declarations
│   ├── gl_capture.cpp         # Capture backend: PBO + fence async path, synchronous path, image flip
│   └── gl_capture.h           # Capture prototypes and callback typedef
├── cmake/
│   ├── Sources.cmake          # Explicit compile list (3 plugin units + 2 SDK units)
│   ├── Dependencies.cmake     # Source-path resolution and FetchContent fallback
│   ├── SteamAPIBridge.cmake   # Configures or fetches the shared bridge subproject
│   ├── LaunchGame.cmake       # Optional F5 deploy support
│   └── VCLTL.cmake            # VC-LTL 5.3.1
├── scripts/
│   ├── build-SteamScreenshots-x86-{Debug,Release}.bat
│   ├── manifests/steamscreenshots.json # Gamedata manifest (one engine global)
│   ├── sync-gamedata.py                # Prunes the upstream catalog into the build tree
│   └── validate-gamedata.py            # Validates it before the plugin target builds
├── tests/
│   ├── capture_tests.cpp      # The 11 OpenGL capture tests (compiles ../src/gl_capture.cpp)
│   ├── support/metahook.h     # Minimal stub header so the tests need no real SDK
│   ├── CMakeLists.txt         # capture_tests target + the CTest case list
│   └── README.md              # Test coverage notes
├── docs/DEPENDENCIES.md       # Dependency versions and notices
├── docs/en/, docs/zh-CN/      # Bilingual pages: installation, build-instruction, tests, debugging
├── memory/project_overview.md # Longer design note (permalink prefix `steamscreenshots/`)
├── thirdparty/cache/          # Ignored VC-LTL binary cache
├── build/x86/<configuration>/    # Ignored build output
├── install/x86/<configuration>/  # Ignored install output
├── README.md, README.zh-CN.md # Bilingual install / compatibility documentation
└── CMakeLists.txt             # Windows MSVC x86 build and install rules
```

## Core Modules

### 1. Plugin lifecycle (`src/plugins.cpp`)

`IPluginsV4` exported through `EXPOSE_SINGLE_INTERFACE(IPluginsV4, IPluginsV4, METAHOOK_PLUGIN_API_VERSION_V4)`:

- `LoadEngine`: collects the file system (`FileSystem`, else `FileSystem_HL25`), engine type and buildnum, copies `cl_enginefunc_t`. **It installs no hooks**
- `LoadClient`: copies the export table, runs `glewInit()`, and **takes over three slots** — `HUD_Frame`, `HUD_Shutdown`, `IN_ActivateMouse`. These are replacements, not inline hooks
- `ExitGame`: `ShutdownSteamBridge()`, `GL_DiscardPendingCapture()`, `UninstallPresentHook()`
- `Shutdown`: `ShutdownSteamBridge()`; both paths release the bridge context idempotently
- `GetVersion`: returns the build timestamp baked in by CMake

The four `mh_dll_info_t` globals declared in `src/plugins.h` are **never defined or used** — leftovers of the address-relocation era. Do not build on them.

### 2. Presentation hooks and Steam integration (`src/exportfuncs.cpp`)

Hook installation is deferred to the **first `IN_ActivateMouse`** call, because the engine registers its `snapshot` command after `HUD_Init`. That first call runs `InstallSDL2Hook()`, `InstallFlipScreenHook()`, and only when a presentation hook is installed and `GL_InitCapture()` succeeded does it register `snapshot` through `HookCmd("snapshot", VID_Snapshot_f)`. It then hooks the `ServerName` message and tries to register the Steam callback.

Two mutually exclusive presentation hooks, in this order:

| Installer | Target | Requirement |
| --- | --- | --- |
| `InstallSDL2Hook` | IAT hook on `SDL2.dll!SDL_GL_SwapWindow` | the engine module imports `SDL2.dll` (`ModuleHasImportEx` + `IATHook`) |
| `InstallFlipScreenHook` | the gamedata `engine` / `VID_FlipScreen` **GLOBAL** | MetaHook API ≥ 109 and a resolvable symbol |

The `VID_FlipScreen` record is the engine's own flip function-pointer slot — `GL_EndRendering` calls through it and it is written once at startup — so swapping the pointer redirects every flip without patching code. The installer saves the previous function in `gPrivateFuncs.Sys_VID_FlipScreen`, writes its own `Sys_VID_FlipScreen` into the slot, and keeps the slot address in `gPrivateFuncs.VID_FlipScreen`. `UninstallPresentHook` restores it.

**Degradation is deliberate and this is the one plugin in this workspace that does *not* fail loudly.** A MetaHook API older than 109, an unresolvable `VID_FlipScreen`, an unusable `SDL2.dll` import or a driver without framebuffer support each log a single `Con_Printf` line and keep the engine's original `snapshot` command.

Capture is split so that the read happens immediately before the frame is presented:

- `VID_Snapshot_f` only calls `GL_RequestCapture()` — it raises the pending flag and returns
- `SDL_GL_SwapWindow` / `Sys_VID_FlipScreen` call `GL_CapturePendingBeforeSwap(ScreenshotCallback)` **before** forwarding to the original flip, so with Renderer enabled its final blit for that frame has already run
- `ScreenshotCallback` hands the buffer to `SteamBridge_WriteScreenshot` on the manager's bridge context; a missing screenshots interface logs once and drops the capture

Steam side (`CSnapshotManager`):

- `TryRegisterCallback` creates the bridge context lazily via `SteamBridge_CreateContext`, and subscribes through `SteamBridge_SubscribeScreenshots` only once `SB_CAP_SCREENSHOTS | SB_CAP_USER` are both available — so a late Steam runtime is picked up rather than failing at load
- `OnSnapshotCallback`: `SB_STEAM_RESULT_OK` → `SteamBridge_SetScreenshotLocation(context, screenshot, g_szServerName)`, then `SteamBridge_GetSteamID` + `SteamBridge_TagScreenshotUser` (logging once if the user interface is unavailable) and `[SteamScreenshots] Snapshot saved.`; `SB_STEAM_RESULT_IO_FAILURE` and every other result get their own messages
- `UnregisterCallback` destroys the context and clears the registered flag

Frames and messages:

- `HUD_Frame` calls the original, re-tries the callback registration, clears `g_szServerName` when the level name is empty (main menu), and then polls the async capture with `GL_QueryAsyncCapture(ScreenshotCallback)`
- `__MsgFunc_ServerName` reads the string with `BEGIN_READ` / `READ_STRING` and stores at most 255 bytes in the 256-byte `g_szServerName`, then forwards to the previously hooked handler
- `HUD_Shutdown` unregisters the callback, shuts the capture backend down, then forwards to the original

### 3. Capture backend (`src/gl_capture.cpp`)

`GL_InitCapture` decides everything once:

- Core/ARB framebuffer support is required (`GLEW_VERSION_3_0 || GLEW_ARB_framebuffer_object` **and** `glBindFramebuffer`) because `GL_READ_FRAMEBUFFER` is used; `EXT_framebuffer_object` alone is not enough
- Asynchronous mode needs OpenGL 3.2 with `glFenceSync` / `glClientWaitSync`; otherwise the synchronous path is selected. The choice is stored in `g_pfnBeginCapture`

`GL_ReadCapturePixels` saves and restores the read-framebuffer binding, the read buffer, all four pack-state values and the PBO binding; it binds framebuffer 0, reads `GL_BACK` as tightly packed `GL_RGB` / `GL_UNSIGNED_BYTE` into either the image buffer (sync) or the bound PBO (async, `pixels == nullptr`).

| | Synchronous (`GL_BeginSyncCapture`) | Asynchronous (`GL_BeginAsyncCapture` + `GL_QueryAsyncCapture`) |
| --- | --- | --- |
| Trigger | the pending flag, before the flip | same, but the read goes to a PBO |
| Completion | immediately: read, flip, callback | the fence is polled once per frame from `HUD_Frame` |
| Re-entrancy | — | a pending fence rejects new requests |
| Cost | can stall for the length of the read | no stall, one frame of latency |

Both paths resize on a video-mode change (`GetVideoMode`), allocate the image buffer as `width * height * 3`, and share the same vertical flip, because `glReadPixels` returns bottom-up rows. The flip loop itself is duplicated in the two functions.

Async specifics: the fence is created with `glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0)`; `GL_QueryAsyncCapture` waits with `glClientWaitSync(..., GL_SYNC_FLUSH_COMMANDS_BIT, 0)`, where `GL_WAIT_FAILED` deletes the sync object and logs `Cannot capture screenshot: GPU fence wait failed.`, while `GL_ALREADY_SIGNALED` / `GL_CONDITION_SATISFIED` map the PBO, copy into the image buffer, flip, invoke the callback, unmap and delete the fence. `GL_ShutdownCapture` drops the pending flag, clears `g_pfnBeginCapture` and frees the image buffer, PBO and sync object.

## Key Code Flow

```
Engine: LoadClient → glewInit, take over HUD_Frame / HUD_Shutdown / IN_ActivateMouse
    ↓
First IN_ActivateMouse
    ↓
InstallSDL2Hook (IAT on SDL2.dll!SDL_GL_SwapWindow)
    └── unavailable → InstallFlipScreenHook (gamedata VID_FlipScreen GLOBAL, API ≥ 109)
            └── unavailable → log once, keep the engine `snapshot` command
    ↓
IsPresentHookInstalled && GL_InitCapture → HookCmd("snapshot", VID_Snapshot_f)
    ↓
User runs `snapshot` → VID_Snapshot_f → GL_RequestCapture (pending flag)
    ↓
Next presentation call → GL_CapturePendingBeforeSwap → GL_BeginCapture
    ├── sync  → glReadPixels, flip, ScreenshotCallback
    └── async → glReadPixels into PBO + glFenceSync
    ↓
Next HUD_Frame → GL_QueryAsyncCapture → map PBO, flip, ScreenshotCallback
    ↓
ScreenshotCallback → SteamBridge_WriteScreenshot
    ↓
Steam: ScreenshotReady_t → OnSnapshotCallback
    └── OK → SetScreenshotLocation(g_szServerName) + GetSteamID + TagScreenshotUser
```

## Build Instructions

Requirements: Windows, Visual Studio 2022 with the C++ desktop workload, CMake 3.21 or newer, Git, Python 3.8 or newer, MSVC x86 (`-A Win32`), C++20 (`cxx_std_20`), static CRT (`MultiThreaded`), static GLEW (`GLEW_STATIC`), `_MBCS` and VC-LTL 5.3.1. No forced AVX2 instruction set.

```bat
scripts\build-SteamScreenshots-x86-Release.bat
scripts\build-SteamScreenshots-x86-Debug.bat
```

The scripts configure, build and install, forwarding extra CMake arguments. Debug compiles at `/W0`, Release at `/W3`; both suppress `/wd4311 /wd4312 /wd4819 /wd4996` and pass `/permissive`. Release also enables interprocedural optimization and `/OPT:REF /OPT:ICF`. Output stays in `build/x86/<configuration>/`; the DLL, its PDB and the gamedata catalog are installed to `install/x86/<configuration>/svencoop/metahook/`. Nothing is deployed to the game automatically.

### Dependencies

| CMake variable | Expected contents | When empty |
| --- | --- | --- |
| `METAHOOK_SOURCE_PATH` | MetaHook root with `include/metahook.h` and the HLSDK sources | pinned FetchContent |
| `STEAMSDK_SOURCE_PATH` | SteamSDK `steam/` headers and `STEAM-SDK-NOTICE.md` (read-only) | pinned FetchContent |
| `STEAMAPIBRIDGE_SOURCE_PATH` | SteamAPIBridge source with `CMakeLists.txt` | pinned commit, built as a shared DLL |
| `GLEW_SOURCE_PATH` | glew-cmake root providing the `libglew_static` target | configure fails |
| `CAPSTONE_INCLUDE_DIRS` | directories containing `capstone.h` | SDK `thirdparty/capstone_fork`, else pinned |
| `GLFW_SOURCE_PATH` | GLFW source root, tests only | pinned FetchContent when `BUILD_TESTING=ON` |
| `VC_LTL_Root` | existing VC-LTL package root | downloaded to `thirdparty/cache`, SHA256-checked |

External source trees are read-only build inputs. GLEW is the only dependency configured and built here (`add_subdirectory(..., EXCLUDE_FROM_ALL)`); SteamAPIBridge is added as a subproject, and **that subproject — not this `CMakeLists.txt` — installs `metahook/dlls/SteamAPIBridge.dll` and the `metahook/licenses/SteamScreenshots/` notices**. The plugin links `libglew_static`, `SteamAPIBridge` and `opengl32`; Capstone is never linked.

Keep `cmake/Sources.cmake` as the explicit compile list (3 plugin units); `include/HLSDK/common/interface.cpp` and `parsemsg.cpp` are compiled in for `EXPOSE_SINGLE_INTERFACE` and the `ServerName` message.

### gamedata

`scripts/manifests/steamscreenshots.json` declares a single `engine` / `VID_FlipScreen` **global**, present in nine upstream snapshots (`cof-5936`, `hl-10210`, `hl-3248`, `hl-3266`, `hl-3329`, `hl-3647`, `hl-4554`, `hl-6153`, `hl-8684`). There are no function or patch records.

`scripts/manifests/steamscreenshots.json` → `scripts/sync-gamedata.py` → pruned catalog under `build/x86/<Configuration>/assets/svencoop/metahook/gamedata/steamscreenshots`, validated by `scripts/validate-gamedata.py` before the plugin target builds. Disable with `-DSTEAMSCREENSHOTS_SYNC_GAMEDATA=OFF`; `STEAMSCREENSHOTS_GAMEDATA_DIR` selects an existing catalog directory. When gamedata usage changes, update the manifest in the same change.

To validate an installed catalog:

```bat
python scripts\validate-gamedata.py install\x86\Release\svencoop\metahook\gamedata\steamscreenshots --manifest scripts\manifests\steamscreenshots.json
```

### Tests

`BUILD_TESTING` defaults to **OFF**. Enabling it fetches pinned GLFW (or uses `GLFW_SOURCE_PATH`) and builds `capture_tests`, which compiles `tests/capture_tests.cpp` together with `../src/gl_capture.cpp` against the stub `tests/support/metahook.h` — no real SDK is involved.

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DBUILD_TESTING=ON
ctest --test-dir build/x86/Release -C Release --output-on-failure
```

11 CTest cases with a 20-second timeout each: `capture.sync-default`, `capture.async-default`, `capture.sync-dirty`, `capture.async-dirty`, `capture.sync-pending`, `capture.async-frame-pending`, `capture.wait-timeout`, `capture.wait-failure`, `capture.framebuffer-unavailable`, `capture.framebuffer-entrypoint-unavailable`, `capture.arb-framebuffer`. Running them requires a desktop OpenGL 3.3 driver. Assertions stay enabled in Release; configuration and documentation text are not assertion targets. GLFW and the test executable are never installed.

### Optional F5 debugging

```powershell
cmake -S . -B build/launch -G "Visual Studio 17 2022" -A Win32 -DMETAHOOKSV_ENABLE_LAUNCH_GAME=ON
```

Select **LaunchGame** and press F5; **DeployGame** builds, stages and copies the plugin DLL/PDB/resources into an existing MetaHook installation before the debugger attaches. The feature defaults OFF. See `docs/en/debugging.md` for `METAHOOKSV_GAME_*` options.

### CI

`.github/workflows/livebuild.yml` runs on main pushes, pull requests and manual dispatch; `release.yml` runs on `v*` tags and packages `SteamScreenshots-windows-x86.7z` (the packaging step excludes the `licenses` directory). Both compile the plugin and the tests and validate the installed gamedata. Capture tests are executed locally with a suitable OpenGL driver, not in CI.

## Engine Compatibility

Capture support depends on which presentation hook the engine allows, not on a buildnum check:

| Engine / build | Support |
| --- | --- |
| `svencoop-10257` (SvEngine) | ✅ SDL2 IAT hook, no gamedata record needed |
| `hl-10210` (GoldSrc HL25) | ✅ `VID_FlipScreen` global — the validated global-hook build |
| Other listed snapshots (`cof-5936`, `hl-3248`…`hl-8684`) | ❓ catalog coverage only — the global exists upstream, but the hook was not validated |
| Engine with neither hook available | ❌ logs once and keeps the engine's own `snapshot` command |

The game must also provide a compatible x86 `steam_api.dll`, initialize Steamworks and dispatch its callbacks; a game without that Steam runtime cannot work with this plugin. Capture additionally requires core/ARB framebuffer support.

## Important Constants, Macros and Types

```cpp
#define MHPluginName "SteamScreenshots"

// src/plugins.h — the presentation hook state
void(__cdecl* SDL_GL_SwapWindow)(void* window);       // saved IAT target
void(__cdecl** VID_FlipScreen)(void);                 // the engine's flip slot
void(__cdecl* Sys_VID_FlipScreen)(void);              // saved value of that slot

// src/exportfuncs.cpp
char g_szServerName[256];                             // capped at 255 bytes, cleared outside a map
static hook_t* g_pPresentHook;                        // non-NULL once the SDL2 hook is installed
// one-shot logging flags: g_bLoggedSteamUserUnavailable,
//                        g_bLoggedSteamScreenshotsWriteUnavailable

// src/gl_capture.cpp
bool g_CapturePending;                                // raised by GL_RequestCapture()
GLuint g_CapturePBO; GLsync g_CaptureSyncObject;      // async path only
bool (*g_pfnBeginCapture)(fnGLQueryCaptureCallback);  // sync or async entry, chosen once

// Capability / result values reported by SteamAPIBridge
SB_CAP_SCREENSHOTS | SB_CAP_USER     // both required before subscribing
SB_STEAM_RESULT_OK, SB_STEAM_RESULT_IO_FAILURE
```

Runtime configuration: `SteamScreenshots.dll` must be listed in the host's `metahook/configs/plugins.lst`, and `SteamAPIBridge.dll` (installed under `metahook/dlls`) must stay next to the plugin.

## Debugging Tips

1. **Console output**: every degradation path logs its own `[SteamScreenshots] …` line — "SDL hook unavailable", "Gamedata API unavailable", "VID_FlipScreen unavailable (<status>)", "Framebuffer capture unavailable", "keeping the engine snapshot command". Two Steam paths log only once per process
2. **Breakpoint locations**: `IN_ActivateMouse()` (deferred installation), `InstallFlipScreenHook()` (the pointer swap), `GL_CapturePendingBeforeSwap()` (the read), `GL_QueryAsyncCapture()` (the fence poll), `OnSnapshotCallback()` (the Steam result)
3. **The capture timestamp matters**: the read must stay before the forward to the original flip, otherwise the back buffer may already have been consumed
4. **Async capture has a one-frame delay** by design; a breakpoint in `ScreenshotCallback` fires from `HUD_Frame`, not from the presentation hook
5. **Tests need a real GL 3.3 driver**; running `capture_tests` headless or over a remote session will fail the framebuffer cases

## Repository Rules

- Preserve the MetaHook API, plugin exports, calling conventions and capture behavior. Match the naming, indentation and comment style of the files you touch
- Resolve engine private symbols only through the host gamedata contract. **Do not** add signature-scan fallbacks for `VID_FlipScreen`; the `Search_Pattern*` macros in `src/plugins.h` are legacy helpers, not a supported path
- Keep the graceful-degradation policy. Unlike the fail-loudly plugins in this workspace, a missing or failed symbol must log once and keep the engine's original `snapshot` command — never `Sys_Error` at load time — and support is decided by the availability of a presentation hook, not by hard-coded buildnum checks
- Keep the deferred installation point (first `IN_ActivateMouse`); the engine registers `snapshot` after `HUD_Init`, so installing earlier would silently lose the takeover
- Do not modify external or third-party sources. MetaHook, SteamSDK, GLEW, Capstone and GLFW are read-only build inputs; only GLEW is configured and built here
- MSVC x86 only. Keep the static CRT / VC-LTL and C++20 (`cxx_std_20`) settings in `CMakeLists.txt` in sync with the other standalone plugin repositories
- When gamedata usage changes, update `scripts/manifests/steamscreenshots.json` in the same change
- Regression tests keep assertions enabled even in Release; documentation and configuration text are not assertion targets
- `README.md` / `README.zh-CN.md` and `docs/en/` / `docs/zh-CN/` are pairs: keep install steps, compatibility claims and build options consistent across both languages
- Verification distinguishes build and simulated tests from a real game run: the capture tests need an OpenGL 3.3 driver, and hook ordering plus Steam submission can only be verified in game. Claims about game or Steam behavior must not be made without evidence. Documentation changes need content, path and format checks, not a plugin rebuild

## Related Links

- **MetaHookSV**: https://github.com/hzqst/MetaHookSv
- **Gamedata symbol catalog**: https://hlnd2t.github.io/GoldSrc_VibeSignatures/
- **Steamworks ISteamScreenshots**: https://partner.steamgames.com/doc/api/ISteamScreenshots
- **GLEW**: https://glew.sourceforge.net/
