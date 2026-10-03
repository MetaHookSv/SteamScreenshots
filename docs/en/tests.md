[Back to README](../../README.md) | [中文](../zh-CN/tests.md)

# Tests

`BUILD_TESTING` defaults to `OFF`. Enable it to build the 11 original OpenGL capture tests.
Running them requires a desktop OpenGL 3.3 driver:

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DBUILD_TESTING=ON
ctest --test-dir build/x86/Release -C Release --output-on-failure
```

Enabling the option fetches the pinned GLFW source unless `GLFW_SOURCE_PATH` is set. GLFW and
the test executable are not installed.

See [test coverage](../../tests/README.md). These tests check actual captured pixels and
OpenGL state; engine hook ordering and Steam submission require game integration testing.
