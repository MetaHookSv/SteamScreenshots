[Back to README](../../README.md) | [中文](../zh-CN/build-instruction.md)

# Build instruction

This page covers building the SteamScreenshots plugin, its dependencies, gamedata and CI.
The install layout and enabling the plugin are in [Installation](installation.md); running
the capture tests is in [Tests](tests.md).

## Requirements

- Windows and Visual Studio 2022 with the C++ desktop workload
- CMake 3.21 or newer
- Git
- Python 3.8 or newer, for gamedata synchronization and validation

The plugin builds for MSVC x86 only, without a forced AVX2 instruction set.

## Build

```bat
scripts\build-SteamScreenshots-x86-Release.bat
scripts\build-SteamScreenshots-x86-Debug.bat
```

The scripts configure under `build/x86/<Configuration>` and install to
`install/x86/<Configuration>/svencoop/metahook`. Installation includes the DLL, PDB, pruned
gamedata and dependency notices. It does not deploy into a game.

The first configure fetches pinned MetaHook, SteamSDK, GLEW and, when needed, Capstone
headers. VC-LTL 5.3.1 is downloaded into `thirdparty/cache` and checked against its SHA256.
The build uses C++20, a static CRT and static GLEW. MetaHook is consumed as an SDK; the
launcher and Capstone library are not built.

## Local dependency paths

Build scripts forward extra CMake arguments. Local dependency paths also accept the same
environment variables:

| CMake variable | Expected contents |
| --- | --- |
| `METAHOOK_SOURCE_PATH` | MetaHook repository root with `include/metahook.h` and HLSDK sources |
| `STEAMSDK_SOURCE_PATH` | SteamSDK `steam/` headers and `STEAM-SDK-NOTICE.md` (read-only) |
| `STEAMAPIBRIDGE_SOURCE_PATH` | SteamAPIBridge source; empty fetches the pinned commit and builds its shared DLL |
| `GLEW_SOURCE_PATH` | glew-cmake root with `CMakeLists.txt` and `include/GL/glew.h` |
| `CAPSTONE_INCLUDE_DIRS` | Include directories containing `capstone.h` or `capstone/capstone.h` |
| `GLFW_SOURCE_PATH` | GLFW source root; used only with `BUILD_TESTING=ON` |
| `VC_LTL_Root` | Existing VC-LTL binary package root |

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DMETAHOOK_SOURCE_PATH=D:\MetaHook -DSTEAMSDK_SOURCE_PATH=D:\SteamSDK
```

Capstone headers resolve from the explicit directories, then the SDK's
`thirdparty/capstone_fork`, then a pinned checkout. External source trees are read-only build
inputs. See [dependency versions and notices](../DEPENDENCIES.md).

## gamedata

Supported builds are `hl-10210` and `svencoop-10257`. The GoldSrc build is hooked through the
Windows engine global `VID_FlipScreen`; Sven Co-op uses the SDL2 presentation hook and does not
require a `VID_FlipScreen` record.

Each build synchronizes and validates a catalog containing only `VID_FlipScreen`. It includes
the nine upstream snapshots where that global exists: `cof-5936`, `hl-10210`, `hl-3248`,
`hl-3266`, `hl-3329`, `hl-3647`, `hl-4554`, `hl-6153` and `hl-8684`. Catalog coverage is not
a list of supported builds; of those, only `hl-10210` is validated. Missing gamedata retains
the original runtime fallback.

Use `-DSTEAMSCREENSHOTS_SYNC_GAMEDATA=OFF` to skip synchronization and provide a compatible
catalog when deploying to engines that use the global hook. `STEAMSCREENSHOTS_GAMEDATA_DIR`
can select an existing catalog directory.

```bat
python scripts\validate-gamedata.py install\x86\Release\svencoop\metahook\gamedata\steamscreenshots --manifest scripts\manifests\steamscreenshots.json
```

## CI

LiveBuild runs on main pushes, pull requests and manual dispatch. Release runs on `v*` tags.
Both compile the plugin and tests, validate installed gamedata, and package
`SteamScreenshots-windows-x86.7z` on Windows 2022. Capture tests are executed locally with a
suitable OpenGL driver.
