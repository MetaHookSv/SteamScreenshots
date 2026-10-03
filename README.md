# SteamScreenshots

[中文文档](README.zh-CN.md)

SteamScreenshots is a MetaHook plugin that sends the engine's `snapshot` command
to the Steam Screenshot Manager. It captures the final OpenGL back buffer before
presentation, flips the RGB image, and adds the server location and Steam user tag.

The plugin retains the implementation from MetaHookSv `fe80b6d`. Its documented
game targets are Sven Co-op and GoldSrc after the Half-Life 25th anniversary
update. The standalone build does not establish runtime testing of every engine.

## Install

1. Install [MetaHook](https://github.com/MetaHookSv/MetaHook).
2. Download `SteamScreenshots-windows-x86.7z` from
   [GitHub Releases](https://github.com/MetaHookSv/SteamScreenshots/releases), or build it locally.
3. Copy the contents of the package's `svencoop/` into your game's mod directory
   (`svencoop/` for Sven Co-op, `valve/` for Half-Life). Keep the plugin and
   gamedata directories together.
4. Add `SteamScreenshots.dll` on its own line in `metahook/configs/plugins.lst`.
5. Launch through MetaHook and run the `snapshot` console command or its bound key.

The game must provide a compatible x86 `steam_api.dll`, initialize Steamworks and
dispatch its callbacks. This package uses that runtime; it does not replace it
or initialize Steam on behalf of the game.

Capture requires core/ARB framebuffer support. OpenGL 3.2 with synchronization
entry points uses asynchronous PBO capture; otherwise capture is synchronous.
When a presentation hook or framebuffer capture is unavailable, the plugin
keeps the engine's original snapshot command. Missing Steam interfaces are logged.

## Build

Requirements: Windows, Visual Studio 2022 with C++ tools, CMake 3.21 or newer,
Git, and Python 3.8 or newer. The plugin builds for MSVC x86 only, without a
forced AVX2 instruction set.

```bat
scripts\build-SteamScreenshots-x86-Release.bat
scripts\build-SteamScreenshots-x86-Debug.bat
```

The scripts configure under `build/x86/<Configuration>` and install to
`install/x86/<Configuration>/svencoop/metahook`. Installation includes the DLL,
PDB, pruned gamedata and dependency notices. It does not deploy into a game.

The first configure fetches pinned MetaHook, SteamSDK, GLEW and, when needed,
Capstone headers. VC-LTL 5.3.1 is downloaded into `thirdparty/cache` and checked
against its SHA256. The build uses C++20, a static CRT and static GLEW. MetaHook
is consumed as an SDK; the launcher and Capstone library are not built.

Build scripts forward extra CMake arguments. Local dependency paths also accept
the same environment variables:

| CMake variable | Expected contents |
| --- | --- |
| `METAHOOK_SOURCE_PATH` | MetaHook repository root with `include/metahook.h` and HLSDK sources |
| `STEAMSDK_SOURCE_PATH` | SteamSDK root with `steam/`, `lib/steam_api.lib` and `bin/steam_api.dll` |
| `GLEW_SOURCE_PATH` | glew-cmake root with `CMakeLists.txt` and `include/GL/glew.h` |
| `CAPSTONE_INCLUDE_DIRS` | Include directories containing `capstone.h` or `capstone/capstone.h` |
| `GLFW_SOURCE_PATH` | GLFW source root; used only with `BUILD_TESTING=ON` |
| `VC_LTL_Root` | Existing VC-LTL binary package root |

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DMETAHOOK_SOURCE_PATH=D:\MetaHook -DSTEAMSDK_SOURCE_PATH=D:\SteamSDK
```

Capstone headers resolve from the explicit directories, then the SDK's
`thirdparty/capstone_fork`, then a pinned checkout. External source trees are
read-only build inputs. See [dependency versions and notices](docs/DEPENDENCIES.md).

## gamedata

Each build synchronizes and validates a catalog containing only the Windows
engine global `VID_FlipScreen`. It includes the nine upstream snapshots where
that global exists: `cof-5936`, `hl-10210`, `hl-3248`, `hl-3266`, `hl-3329`,
`hl-3647`, `hl-4554`, `hl-6153`, and `hl-8684`.

Sven Co-op uses the SDL2 presentation hook and does not require a
`VID_FlipScreen` record. Missing gamedata retains the original runtime fallback;
catalog coverage is not a list of newly supported games.

Use `-DSTEAMSCREENSHOTS_SYNC_GAMEDATA=OFF` to skip synchronization and provide a
compatible catalog when deploying to engines that use the global hook.
`STEAMSCREENSHOTS_GAMEDATA_DIR` can select an existing catalog directory.

```bat
python scripts\validate-gamedata.py install\x86\Release\svencoop\metahook\gamedata\steamscreenshots --manifest scripts\manifests\steamscreenshots.json
```

## Tests and CI

`BUILD_TESTING` defaults to `OFF`. Enable it to build the 11 original OpenGL
capture tests. Running them requires a desktop OpenGL 3.3 driver:

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DBUILD_TESTING=ON
ctest --test-dir build/x86/Release -C Release --output-on-failure
```

See [test coverage](tests/README.md). These tests check actual captured pixels and
OpenGL state; engine hook ordering and Steam submission require game integration
testing.

LiveBuild runs on main pushes, pull requests and manual dispatch. Release runs
on `v*` tags. Both compile the plugin and tests, validate installed gamedata,
and package `SteamScreenshots-windows-x86.7z` on Windows 2022. Capture tests are
executed locally with a suitable OpenGL driver.

## License

Plugin code is covered by the [MIT License](LICENSE). Each dependency retains
its own license or applicable terms; local builds install notices under
`metahook/licenses/SteamScreenshots`. The 7z archive excludes `licenses/`.
