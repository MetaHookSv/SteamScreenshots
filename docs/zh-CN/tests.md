[返回 README](../../README.md) | [English](../en/tests.md)

# 测试说明

`BUILD_TESTING` 默认关闭。开启后构建原有 11 项捕获测试；执行需要桌面 OpenGL 3.3 驱动：

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DBUILD_TESTING=ON
ctest --test-dir build/x86/Release -C Release --output-on-failure
```

开启该选项后会获取固定提交的 GLFW 源码，除非设置了 `GLFW_SOURCE_PATH`。GLFW 和测试可执行
文件不随插件安装。

测试检查真实捕获像素及 OpenGL 状态，详见[捕获测试说明](../../tests/README.md)。引擎 Hook
顺序和 Steam 提交仍需游戏内集成验证。
