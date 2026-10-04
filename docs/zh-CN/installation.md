[返回 README](../../README.md) | [English](../en/installation.md)

# 安装说明

`install/x86/<Debug|Release>/`：

```text
svencoop/
  metahook/plugins/SteamScreenshots.dll
  metahook/plugins/SteamScreenshots.pdb
  metahook/dlls/SteamAPIBridge.dll
  metahook/dlls/SteamAPIBridge.pdb
  metahook/gamedata/steamscreenshots/   (SteamScreenshots 自己的 gamedata json)
  metahook/licenses/SteamScreenshots/   (依赖许可说明，7z 包中不包含)
```

1. 安装 [MetaHook](https://github.com/MetaHookSv/MetaHook)。
2. 从 [GitHub Release](https://github.com/MetaHookSv/SteamScreenshots/releases) 下载
   `SteamScreenshots-windows-x86.7z`，或自行构建。
3. 将压缩包中 `svencoop/` 的内容复制到游戏的 mod 目录：Sven Co-op 使用 `svencoop/`，
   Half-Life 使用 `valve/`。保留插件和 gamedata 目录。
4. 在 `metahook/configs/plugins.lst` 中单独添加一行 `SteamScreenshots.dll`。
5. 通过 MetaHook 启动游戏，执行 `snapshot` 命令或相应的绑定按键。

## 游戏运行时

游戏需要提供兼容的 x86 `steam_api.dll`，初始化 Steamworks 并派发回调。安装包使用游戏
已有的 runtime，不替换该 DLL，也不代替游戏初始化 Steam。

## 截图行为

截图需要 core/ARB framebuffer 支持。OpenGL 3.2 且同步入口可用时采用 PBO 异步读取，
否则采用同步读取。

呈现 Hook 或 framebuffer 捕获不可用时，保留引擎原截图命令；Steam 接口缺失时输出日志。
