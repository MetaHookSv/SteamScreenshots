[Back to README](../../README.md) | [中文](../zh-CN/installation.md)

# Installation

`install/x86/<Debug|Release>/`:

```text
svencoop/
  metahook/plugins/SteamScreenshots.dll
  metahook/plugins/SteamScreenshots.pdb
  metahook/dlls/SteamAPIBridge.dll
  metahook/dlls/SteamAPIBridge.pdb
  metahook/gamedata/steamscreenshots/   (SteamScreenshots' own gamedata json)
  metahook/licenses/SteamScreenshots/   (dependency notices, excluded from the 7z)
```

1. Install [MetaHook](https://github.com/MetaHookSv/MetaHook).
2. Download `SteamScreenshots-windows-x86.7z` from
   [GitHub Releases](https://github.com/MetaHookSv/SteamScreenshots/releases), or build it
   locally.
3. Copy the contents of the package's `svencoop/` into your game's mod directory
   (`svencoop/` for Sven Co-op, `valve/` for Half-Life). Keep the plugin and gamedata
   directories together.
4. Add `SteamScreenshots.dll` on its own line in `metahook/configs/plugins.lst`.
5. Launch through MetaHook and run the `snapshot` console command or its bound key.

## Game runtime

The game must provide a compatible x86 `steam_api.dll`, initialize Steamworks and dispatch
its callbacks. This package uses that runtime; it does not replace it or initialize Steam on
behalf of the game.

## Capture behavior

Capture requires core/ARB framebuffer support. OpenGL 3.2 with synchronization entry points
uses asynchronous PBO capture; otherwise capture is synchronous.

When a presentation hook or framebuffer capture is unavailable, the plugin keeps the engine's
original snapshot command. Missing Steam interfaces are logged.
