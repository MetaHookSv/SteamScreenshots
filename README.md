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

## License

Licensed under the [MIT License](LICENSE); each dependency keeps its own license.
