# SteamScreenshots

This plugin dynamically links SteamAPIBridge.dll, installed under `metahook/dlls`.
Keep this dependency with the plugin when deploying. The bridge uses the game's
existing Steam runtime; it does not replace `steam_api.dll`. Standalone builds accept
`STEAMAPIBRIDGE_SOURCE_PATH` or fetch a fixed bridge commit.

[中文文档](README.zh-CN.md)

SteamScreenshots is a MetaHook plugin that takes over GoldSrc engine's `snapshot` command and
sends captured image to the Steam Screenshot Manager.

* The game must provide a compatible x86 `steam_api.dll`, initialize Steamworks and dispatch
  its callbacks. thus non-Steam (pirate) games won't be working with this plugin.
* Capture requires core/ARB framebuffer support. OpenGL 3.2 with synchronization entry points
  uses asynchronous PBO capture; otherwise capture is synchronous.
* When a presentation hook or framebuffer capture is unavailable, the engine's original
  snapshot command is kept. Missing Steam interfaces are logged.

## Compatibility

| Engine | |
| --- | --- |
| GoldSrc_blob (3248~4554) | ? (not tested) |
| GoldSrc_legacy (< 6153) | ? (not tested) |
| GoldSrc_new (8684 ~) | ? (not tested) |
| SvEngine (8832 ~) | √ |
| GoldSrc_HL25 (>= 9884) | √ |

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
- [F5 debugging (optional)](docs/en/debugging.md)
- [Dependency versions and notices](docs/DEPENDENCIES.md)

## License

Licensed under the [MIT License](LICENSE); each dependency keeps its own license.
