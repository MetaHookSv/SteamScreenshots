# SteamScreenshots

[English](README.md)

SteamScreenshots 是 MetaHook 截图插件，将引擎的 `snapshot` 命令接入 Steam
截图管理器。插件在呈现前读取最终 OpenGL 后缓冲，翻转 RGB 图像，并附加服务器
位置及 Steam 用户标签。

截图实现沿用 MetaHookSv `fe80b6d`。原项目注明的游戏范围为 Sven Co-op 和
Half-Life 25 周年更新后的 GoldSrc；独立构建不代表已对所有引擎完成运行验证。

## 安装

1. 安装 [MetaHook](https://github.com/MetaHookSv/MetaHook)。
2. 从 [Releases](https://github.com/MetaHookSv/SteamScreenshots/releases) 下载
   `SteamScreenshots-windows-x86.7z`，或自行构建。
3. 将压缩包中 `svencoop/` 的内容复制到游戏的 mod 目录：Sven Co-op 使用
   `svencoop/`，Half-Life 使用 `valve/`。保留插件和 gamedata 目录。
4. 在 `metahook/configs/plugins.lst` 中单独添加一行 `SteamScreenshots.dll`。
5. 通过 MetaHook 启动游戏，执行 `snapshot` 命令或相应的绑定按键。

游戏需要提供兼容的 x86 `steam_api.dll`，初始化 Steamworks 并派发回调。
安装包使用游戏已有的 runtime，不替换该 DLL，也不代替游戏初始化 Steam。

截图需要 core/ARB framebuffer 支持。OpenGL 3.2 且同步入口可用时采用 PBO
异步读取，否则采用同步读取。呈现 Hook 或 framebuffer 捕获不可用时，保留
引擎原截图命令；Steam 接口缺失时输出日志。

## 构建

要求 Windows、安装 C++ 工具的 Visual Studio 2022、CMake 3.21+、Git 和
Python 3.8+。仅支持 MSVC x86，Release 不强制使用 AVX2。

```bat
scripts\build-SteamScreenshots-x86-Release.bat
scripts\build-SteamScreenshots-x86-Debug.bat
```

构建目录为 `build/x86/<Configuration>`，安装目录为
`install/x86/<Configuration>/svencoop/metahook`，包含 DLL、PDB、裁剪后的
gamedata 和依赖许可说明。构建脚本不直接部署到游戏。

首次配置自动获取固定提交的 MetaHook、SteamSDK、GLEW 和必要的 Capstone
headers，并下载 SHA256 校验的 VC-LTL 5.3.1 到 `thirdparty/cache`。使用
C++20、静态 CRT 和静态 GLEW；MetaHook 作为 SDK 使用，不构建 launcher
或链接 Capstone。

脚本透传额外 CMake 参数。以下变量也支持同名环境变量：

| CMake 变量 | 路径要求 |
| --- | --- |
| `METAHOOK_SOURCE_PATH` | 含 `include/metahook.h` 和 HLSDK 源码的 MetaHook 根目录 |
| `STEAMSDK_SOURCE_PATH` | 含 `steam/`、`lib/steam_api.lib`、`bin/steam_api.dll` 的 SteamSDK 根目录 |
| `GLEW_SOURCE_PATH` | 含 `CMakeLists.txt`、`include/GL/glew.h` 的 glew-cmake 根目录 |
| `CAPSTONE_INCLUDE_DIRS` | 含 `capstone.h` 或 `capstone/capstone.h` 的 include 目录列表 |
| `GLFW_SOURCE_PATH` | GLFW 根目录，仅在开启测试时使用 |
| `VC_LTL_Root` | 已有 VC-LTL 二进制包根目录 |

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DMETAHOOK_SOURCE_PATH=D:\MetaHook -DSTEAMSDK_SOURCE_PATH=D:\SteamSDK
```

Capstone headers 优先使用显式目录，然后使用 SDK 的
`thirdparty/capstone_fork`，最后获取固定提交。外部源码目录作为只读输入。
依赖版本及许可见 [DEPENDENCIES](docs/DEPENDENCIES.md)。

## gamedata

每次构建同步并校验仅含 Windows 引擎 global `VID_FlipScreen` 的 catalog。
清单包含提供该符号的九个上游快照：`cof-5936`、`hl-10210`、`hl-3248`、
`hl-3266`、`hl-3329`、`hl-3647`、`hl-4554`、`hl-6153`、`hl-8684`。

Sven Co-op 使用 SDL2 呈现 Hook，不依赖 `VID_FlipScreen` 记录。gamedata
缺失时保留原有运行时回退；catalog 覆盖范围不表示新增游戏支持。

使用 `-DSTEAMSCREENSHOTS_SYNC_GAMEDATA=OFF` 跳过同步；部署到使用 global
Hook 的引擎时应自行提供兼容 catalog。可通过 `STEAMSCREENSHOTS_GAMEDATA_DIR`
选择已有 catalog 目录。

```bat
python scripts\validate-gamedata.py install\x86\Release\svencoop\metahook\gamedata\steamscreenshots --manifest scripts\manifests\steamscreenshots.json
```

## 测试与 CI

`BUILD_TESTING` 默认关闭。开启后构建原有 11 项捕获测试；执行需要桌面
OpenGL 3.3 驱动：

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DBUILD_TESTING=ON
ctest --test-dir build/x86/Release -C Release --output-on-failure
```

测试检查真实捕获像素及 OpenGL 状态，详见 [测试说明](tests/README.md)。
引擎 Hook 顺序和 Steam 提交仍需游戏内集成验证。

LiveBuild 响应 main 分支 push、PR 和手动触发；Release 响应 `v*` 标签。
两者使用 Windows 2022 runner 编译插件及测试，校验安装后的 gamedata，并
打包 `SteamScreenshots-windows-x86.7z`。捕获测试在具备驱动的本机执行。

## 许可证

插件代码使用 [MIT License](LICENSE)。依赖保留各自许可证或适用条款；
本地构建安装后的说明位于 `metahook/licenses/SteamScreenshots`，7z 压缩包
不包含 `licenses/` 目录。
