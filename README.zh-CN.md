# SteamScreenshots

本插件动态链接 `SteamAPIBridge.dll`，安装在 `metahook/dlls`。部署插件时请一并保留该依赖。
Bridge 使用游戏已有的 Steam 运行库，不替换 `steam_api.dll`。独立构建可通过
`STEAMAPIBRIDGE_SOURCE_PATH` 指定源码，否则获取固定提交。

[English README](README.md)

SteamScreenshots 是 MetaHook 截图插件，本插件会接管引擎的 `snapshot` 命令，并将截图画面
发送给 Steam 截图管理器。

* 游戏需要提供兼容的 x86 `steam_api.dll`，初始化 Steamworks 并派发回调。因此该插件不会在non-Steam(盗版)的游戏上生效。
* 截图需要 core/ARB framebuffer 支持。OpenGL 3.2 且同步入口可用时采用 PBO 异步读取，
  否则采用同步读取。
* 呈现 Hook 或 framebuffer 捕获不可用时，保留引擎原截图命令；Steam 接口缺失时输出日志。

## 兼容性

## Compatibility

| Engine | |
| --- | --- |
| GoldSrc_blob (3248~4554) | ? (not tested) |
| GoldSrc_legacy (< 6153) | ? (not tested) |
| GoldSrc_new (8684 ~) | ? (not tested) |
| SvEngine (8832 ~) | √ |
| GoldSrc_HL25 (>= 9884) | √ |

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
- [F5 调试（可选）](docs/zh-CN/debugging.md)
- [依赖版本及许可](docs/DEPENDENCIES.md)

## 许可证

项目采用 [MIT License](LICENSE)；各依赖保留自己的许可证。
