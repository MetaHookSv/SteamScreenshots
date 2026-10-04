# SteamScreenshots

[中文文档](README.zh-CN.md)

SteamScreenshots is a MetaHook plugin that takes over the engine's `snapshot` command and
sends captured image to the Steam Screenshot Manager.

* The game must provide a compatible x86 `steam_api.dll`, initialize Steamworks and dispatch
  its callbacks.
* Capture requires core/ARB framebuffer support. OpenGL 3.2 with synchronization entry points
  uses asynchronous PBO capture; otherwise capture is synchronous.
* When a presentation hook or framebuffer capture is unavailable, the engine's original
  snapshot command is kept. Missing Steam interfaces are logged.

## Compatibility

|        Engine                     |      |
|        ----                       | ---- |
| GoldSrc_HL25   (hl-10210)         | √    |
| SvEngine       (svencoop-10257)   | √    |

`hl-10210` is hooked through the engine's `VID_FlipScreen` global; `svencoop-10257` uses the
SDL2 presentation hook. The gamedata catalog also lists eight other upstream snapshots where
that global exists; they are catalog coverage, not validated support. Without a record the
original snapshot command is kept.

The plugin builds for MSVC x86 only.

## Quick start

Download `SteamScreenshots-windows-x86.7z` from
[GitHub Releases](https://github.com/MetaHookSv/SteamScreenshots/releases) (built on `v*` tag
pushes).

Merge the extracted `svencoop/` into the target mod directory (`svencoop/` for Sven Co-op,
`valve/` for Half-Life) and add `SteamScreenshots.dll` on its own line in MetaHook's
`metahook/configs/plugins.lst`. Don't forget to launch game from MetaHook.

## Documentation

- [Installation](docs/en/installation.md)
- [Build instruction](docs/en/build-instruction.md)
- [Tests](docs/en/tests.md)
- [Dependency versions and notices](docs/DEPENDENCIES.md)

## F5 debugging (optional)

Install MetaHook and enable this plugin in the game's `plugins.lst` first. Configure a standalone Visual Studio Win32 solution:

```powershell
cmake -S . -B build/launch -G "Visual Studio 17 2022" -A Win32 -DMETAHOOKSV_ENABLE_LAUNCH_GAME=ON
```

Open the solution, select **LaunchGame** and press **F5**. **DeployGame** builds this plugin and its dependencies, stages Install, and copies plugin DLLs/PDBs/resources before the native debugger starts the existing game launcher. Root launchers/runtime files and plugin lists remain unchanged. Set VS to build before running and **Do not launch** on build errors; stop the game before redeploying. Ordinary builds do not deploy.

`METAHOOKSV_GAME_DIRECTORY` defaults to Steam discovery; `METAHOOKSV_GAME_APPID` defaults to `225840`. Set `METAHOOKSV_GAME_MOD` for a custom mod and `METAHOOKSV_GAME_ARGUMENTS` for extra arguments. Debug and Release are supported.

The shared module uses `METAHOOKSV_LAUNCH_GAME_MODULE_DIR`, the surrounding MetaHookSv checkout, or a pinned source archive. Without Installer sources, it downloads the self-contained CLI from GitHub `latest` (no .NET required); `METAHOOKSV_INSTALLER_RELEASE` selects a fixed tag, and `METAHOOKSV_INSTALLER_CLI_EXECUTABLE` supplies an offline EXE. Plugin mode requires v20261004c or later. Valid caches under `build/launch/launch-game/installer/<release>` are reused without update checks; select another tag or clear that private cache to upgrade. `GH_TOKEN`/`GITHUB_TOKEN` may be supplied through the environment if GitHub API rate limits prevent the first download. The feature defaults OFF and performs no extra downloads when disabled.

## License

Licensed under the [MIT License](LICENSE); each dependency keeps its own license.
