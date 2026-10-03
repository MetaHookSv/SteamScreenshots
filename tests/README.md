# OpenGL capture tests

The original 11 C++20 tests compile production `src/gl_capture.cpp` against GLFW
and the same static GLEW used by the plugin. They do not load MetaHook, Steam or
the game. `support/metahook.h` supplies only dimensions and console logging.
Pixels are rendered into a hidden window's back buffer and checked byte for byte.

Run from the repository root with a working desktop OpenGL 3.3 driver:

```bat
scripts\build-SteamScreenshots-x86-Release.bat -DBUILD_TESTING=ON
ctest --test-dir build/x86/Release -C Release --output-on-failure
scripts\build-SteamScreenshots-x86-Debug.bat -DBUILD_TESTING=ON
ctest --test-dir build/x86/Debug -C Debug --output-on-failure
```

`BUILD_TESTING` defaults to `OFF`. Enabling it fetches the pinned GLFW source
unless `GLFW_SOURCE_PATH` is set. GLFW and the test executable are not installed.
OpenGL context creation failures fail the tests rather than silently skipping.

Coverage includes synchronous/asynchronous RGB readback from `GL_BACK` even when
the default read buffer is `GL_FRONT`, restoration of default and caller FBO
read buffers, pixel packing, a pre-bound caller PBO, independent read/draw FBOs,
pending request coalescing and busy PBO retry, asynchronous frame ownership,
capture dimension changes, vertical flipping, and framebuffer capability gates.

The capture region changes inside a fixed hidden framebuffer to avoid
platform-specific hidden-window resize behavior. Fence tests replace GLEW's wait
entry point to inject timeout/failure results; PBOs, readback, fences and recovery
use real OpenGL. Capability tests override capability flags or the bind entry
point; they do not emulate an entire legacy driver.

CI compiles these tests. Execute them locally on a machine providing OpenGL 3.3.
The tests do not validate engine hook ordering, presentation timing or Steam
screenshot submission. Those require game integration testing.

## Port verification (2026-10-03)

Verified on Windows with Visual Studio 2022, MSVC 19.44.35228, CMake 3.31.12
and Python 3.12.5:

- Release fetched every pinned source dependency and the verified VC-LTL package;
  Debug built against explicit local MetaHook, SteamSDK, GLEW, GLFW and Capstone paths.
- Both configurations built and installed successfully; all 11 capture tests
  passed in each configuration (22 executions).
- Both installed DLLs are x86, export CreateInterface, and loaded successfully
  with the matching Steam runtime in an isolated x86 process. The V4 factory
  returned a plugin instance without initializing the engine or Steam.
- Both installed gamedata catalogs passed validation for all nine snapshots.
- Workflow actionlint and composite action YAML parsing passed. The Release
  7z passed integrity checking and contains 18 files: DLL/PDB, catalog and notices.
- All six production files matched the original SHA256; both gamedata helpers
  matched ResourceReplacer. The default configuration with tests and gamedata
  synchronization disabled also configured successfully.

Game integration and GitHub-hosted workflow execution were not performed.
