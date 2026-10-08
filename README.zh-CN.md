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

## C/C++ 格式化

使用 [MetaHookSv/FormatValidation](https://github.com/MetaHookSv/FormatValidation)
共享工具及固定版本 **clang-format 23.1.3**，采用 DiligentCore 风格（4 空格，保留
include 顺序）。为 CMake 使用的 Python 解释器安装格式工具：

```sh
python -m pip install clang-format==23.1.3
cmake -S . -B build/format "-DFORMAT_VALIDATION_ONLY=ON"
cmake --build build/format --target format-check
cmake --build build/format --target format
```

格式专用配置需要 CMake 3.21+、Git、Python 3.9+（CI 使用 3.12）及构建生成器；
使用 `-G Ninja` 可无需 Visual Studio。它不准备原生 SDK 或游戏依赖。
格式目标需显式执行，不加入普通 DLL 构建。使用 Visual Studio 生成器时，执行目标
需追加 `--config Debug` 或 `--config Release`。

聚合仓库注入 `FORMAT_VALIDATION_SOURCE_PATH=thirdparty/FormatValidation`。
独立组件支持该 CMake 参数及同名环境变量；为空时通过 FetchContent 获取固定工具
提交。相对路径应加引号，例如
`"-DFORMAT_VALIDATION_SOURCE_PATH=../../thirdparty/FormatValidation"`。
配置时在仓库根目录生成被 gitignore 的 `.clang-format` 供编辑器使用；格式规则应
在共享仓库修改，不修改生成副本。可通过 `FORMAT_VALIDATION_CLANG_FORMAT_EXECUTABLE`
指定工具路径，但版本仍须与固定版本一致。

检查覆盖 `src/`、`include/`、`tests/` 中维护的 C/C++ 文件，包括未被 Git 忽略的新文件。
相对仓库根目录的排除规则位于 `.clang-format-ignore`；第三方源和构建产物不纳入检查。
`clang-format` workflow 在 push、pull request 和手动运行时执行全量检查。
