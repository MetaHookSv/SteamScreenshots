[返回 README](../../README.md) | [English](../en/build-instruction.md)

# 构建说明

本页涵盖 SteamScreenshots 插件的构建、依赖、gamedata 与 CI 细节。安装目录布局与启用插件
见[安装说明](installation.md)；捕获测试的执行见[测试说明](tests.md)。

## 依赖要求

- Windows、安装 C++ 桌面开发工具的 Visual Studio 2022
- CMake 3.21+
- Git
- Python 3.8+，用于 gamedata 同步与校验

仅支持 MSVC x86，Release 不强制使用 AVX2。

## 构建

```bat
scripts\build-SteamScreenshots-x86-Release.bat
scripts\build-SteamScreenshots-x86-Debug.bat
```

构建目录为 `build/x86/<Configuration>`，安装目录为
`install/x86/<Configuration>/svencoop/metahook`，包含 DLL、PDB、裁剪后的 gamedata 和依赖
许可说明。构建脚本不直接部署到游戏。

首次配置自动获取固定提交的 MetaHook、SteamSDK、GLEW 和必要的 Capstone headers，并下载
SHA256 校验的 VC-LTL 5.3.1 到 `thirdparty/cache`。使用 C++20、静态 CRT 和静态 GLEW；
MetaHook 作为 SDK 使用，不构建 launcher 或链接 Capstone。

## 本地依赖路径

脚本透传额外 CMake 参数。以下变量也支持同名环境变量：

| CMake 变量 | 路径要求 |
| --- | --- |
| `METAHOOK_SOURCE_PATH` | 含 `include/metahook.h` 和 HLSDK 源码的 MetaHook 根目录 |
| `STEAMSDK_SOURCE_PATH` | SteamSDK `steam/` headers and `STEAM-SDK-NOTICE.md` (read-only) |
| `STEAMAPIBRIDGE_SOURCE_PATH` | SteamAPIBridge source; empty fetches the pinned commit and builds its shared DLL |
| `GLEW_SOURCE_PATH` | 含 `CMakeLists.txt`、`include/GL/glew.h` 的 glew-cmake 根目录 |
| `CAPSTONE_INCLUDE_DIRS` | 含 `capstone.h` 或 `capstone/capstone.h` 的 include 目录列表 |
| `GLFW_SOURCE_PATH` | GLFW 根目录，仅在开启测试时使用 |
| `VC_LTL_Root` | 已有 VC-LTL 二进制包根目录 |

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DMETAHOOK_SOURCE_PATH=D:\MetaHook -DSTEAMSDK_SOURCE_PATH=D:\SteamSDK
```

Capstone headers 优先使用显式目录，然后使用 SDK 的 `thirdparty/capstone_fork`，最后获取
固定提交。外部源码目录作为只读输入。依赖版本及许可见[依赖说明](../DEPENDENCIES.md)。

## gamedata

支持的构建为 `hl-10210` 与 `svencoop-10257`。GoldSrc 构建通过 Windows 引擎 global
`VID_FlipScreen` 接管；Sven Co-op 使用 SDL2 呈现 Hook，不依赖 `VID_FlipScreen` 记录。

每次构建同步并校验仅含 `VID_FlipScreen` 的 catalog。清单包含提供该符号的九个上游快照：
`cof-5936`、`hl-10210`、`hl-3248`、`hl-3266`、`hl-3329`、`hl-3647`、`hl-4554`、
`hl-6153`、`hl-8684`。catalog 覆盖范围不表示这些构建均已支持，其中仅 `hl-10210` 已验证。
gamedata 缺失时保留原有运行时回退。

使用 `-DSTEAMSCREENSHOTS_SYNC_GAMEDATA=OFF` 跳过同步；部署到使用 global Hook 的引擎时
应自行提供兼容 catalog。可通过 `STEAMSCREENSHOTS_GAMEDATA_DIR` 选择已有 catalog 目录。

```bat
python scripts\validate-gamedata.py install\x86\Release\svencoop\metahook\gamedata\steamscreenshots --manifest scripts\manifests\steamscreenshots.json
```

## CI

LiveBuild 响应 main 分支 push、PR 和手动触发；Release 响应 `v*` 标签。两者使用 Windows
2022 runner 编译插件及测试，校验安装后的 gamedata，并打包
`SteamScreenshots-windows-x86.7z`。捕获测试在具备驱动的本机执行。
