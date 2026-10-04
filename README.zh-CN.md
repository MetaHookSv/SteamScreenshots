# SteamScreenshots

本插件动态链接 `SteamAPIBridge.dll`，安装在 `metahook/dlls`。部署插件时请一并保留该依赖。
Bridge 使用游戏已有的 Steam 运行库，不替换 `steam_api.dll`。独立构建可通过
`STEAMAPIBRIDGE_SOURCE_PATH` 指定源码，否则获取固定提交。

[English README](README.md)

SteamScreenshots 是 MetaHook 截图插件，本插件会接管引擎的 `snapshot` 命令，并将截图画面
发送给 Steam 截图管理器。

* 游戏需要提供兼容的 x86 `steam_api.dll`，初始化 Steamworks 并派发回调。
* 截图需要 core/ARB framebuffer 支持。OpenGL 3.2 且同步入口可用时采用 PBO 异步读取，
  否则采用同步读取。
* 呈现 Hook 或 framebuffer 捕获不可用时，保留引擎原截图命令；Steam 接口缺失时输出日志。

## 兼容性

|        Engine                     |      |
|        ----                       | ---- |
| GoldSrc_HL25   (hl-10210)         | √    |
| SvEngine       (svencoop-10257)   | √    |

`hl-10210` 通过引擎 global `VID_FlipScreen` 接管；`svencoop-10257` 使用 SDL2 呈现 Hook。
gamedata catalog 另含存在该 global 的八个上游快照，它们只是 catalog 覆盖范围，不表示已验证
支持。缺少记录时保留引擎原截图命令。

插件仅支持 MSVC x86。

## 快速开始

从 [GitHub Release](https://github.com/MetaHookSv/SteamScreenshots/releases) 下载
`SteamScreenshots-windows-x86.7z`（推送 `v*` 标签时构建）。

将解压出的 `svencoop/` 合并到对应 mod 目录（Sven Co-op 使用 `svencoop/`，Half-Life 使用
`valve/`），在 MetaHook 的 `metahook/configs/plugins.lst` 中单独添加一行
`SteamScreenshots.dll`。最后，别忘了从 MetaHook 启动游戏。

## 文档

- [安装说明](docs/zh-CN/installation.md)
- [构建说明](docs/zh-CN/build-instruction.md)
- [测试说明](docs/zh-CN/tests.md)
- [依赖版本及许可](docs/DEPENDENCIES.md)

## F5 调试（可选）

先安装 MetaHook，并在游戏的 `plugins.lst` 中启用本插件，然后配置独立的 Visual Studio Win32 解决方案：

```powershell
cmake -S . -B build/launch -G "Visual Studio 17 2022" -A Win32 -DMETAHOOKSV_ENABLE_LAUNCH_GAME=ON
```

打开解决方案，选择 **LaunchGame** 后按 **F5**。**DeployGame** 编译本插件及依赖，暂存 Install，再复制插件 DLL、PDB 和资源，最后由原生调试器启动已有游戏 launcher。不会修改根目录启动器/运行库或插件列表。VS 应开启运行前构建，并将构建失败策略设为 **不启动**；重新部署前请退出游戏。普通构建不会部署。

`METAHOOKSV_GAME_DIRECTORY` 默认通过 Steam 自动查找，`METAHOOKSV_GAME_APPID` 默认为 `225840`。自定义 mod 使用 `METAHOOKSV_GAME_MOD`，附加参数使用 `METAHOOKSV_GAME_ARGUMENTS`；支持 Debug 和 Release。

共享模块依次从 `METAHOOKSV_LAUNCH_GAME_MODULE_DIR`、所在 MetaHookSv 聚合仓库或固定提交的源码包获取。缺少 Installer 源码时自动下载 GitHub `latest` 的自包含 CLI，无需安装 .NET；可用 `METAHOOKSV_INSTALLER_RELEASE` 固定 tag，或用 `METAHOOKSV_INSTALLER_CLI_EXECUTABLE` 指定离线 EXE。插件模式要求 v20261004c 或之后版本。`build/launch/launch-game/installer/<release>` 下的有效缓存直接复用，不自动升级；切换 tag 或清理该私有缓存后重新下载。首次下载若触发 GitHub API 限流，可通过环境变量 `GH_TOKEN`/`GITHUB_TOKEN` 提供凭据。功能默认 OFF，关闭时不新增下载。

## 许可证

项目采用 [MIT License](LICENSE)；各依赖保留自己的许可证。
