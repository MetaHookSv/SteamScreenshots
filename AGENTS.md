# AGENTS.md

This file provides guidance and important rules working with code in this repository.

## When coding / building plan

- Use a progressive disclosure approach for agent coding in this repository: start from high-level
  information in the Basic Memory knowledge base first, and only locate/read specific files or
  symbols when necessary, instead of expanding a large amount of context at once.

#### Basic Memory knowledge base (project-scoped, `memory/`)

- Notes live in `memory/` (markdown with YAML frontmatter: `title`/`type`/`permalink`), tracked in git.
- This repository contains the standalone SteamScreenshots plugin, extracted from MetaHookSv
  `Plugins/SteamScreenshots`. Its notes were migrated from MetaHookSv and adapted to the CMake
  workspace; see `memory/project_overview.md` for scope and provenance.
- Basic Memory is registered as MCP server `basic-memory`, pinned to the `steamscreenshots` project
  (project-level `.mcp.json`, mirrored by `.codex/config.toml`). The `metahooksv` project belongs to
  the source repository.
- Prefer Basic Memory MCP tools (`search_notes` / `read_note` / `write_note` / `edit_note`) only when
  their project resolves to this repository's `memory/` directory. Verify the project binding before
  writing; when no matching project is available, read and edit the local markdown files directly.
- Notes use the `steamscreenshots/` permalink prefix to distinguish them from the source repository.
- Historical records are not current evidence: the migrated note retains MetaHookSv paths and the
  old snapshot-path capture design, while the current sources are `src/<file>` and capture now runs
  at the presentation call. Do not extend an old statement to a new change without checking the code.

#### High-level information in this repository (read corresponding notes first)

- Project overview, provenance, dependency boundaries and entry points: `project_overview`

#### When notes are insufficient: source entry points (query and read on demand)

- Build: `CMakeLists.txt`, `cmake/Sources.cmake` (explicit compile list), `cmake/Dependencies.cmake`
  (source-path resolution and FetchContent fallback), `cmake/VCLTL.cmake`,
  `scripts/build-SteamScreenshots-x86-{Debug,Release}.bat`
- Plugin sources: `src/`; lifecycle entry `src/plugins.cpp`, hooks and Steam callbacks
  `src/exportfuncs.cpp`, capture backend `src/gl_capture.cpp`, shared globals `src/plugins.h`
- Public API / interface: MetaHook's `include/metahook.h` and `include/Interface/`, consumed as an
  SDK and never built here
- gamedata: `scripts/manifests/steamscreenshots.json` (only `engine`/`VID_FlipScreen`/`global`),
  `scripts/sync-gamedata.py`, `scripts/validate-gamedata.py`; the build-time sync prunes the upstream
  catalog into the nested `metahook/gamedata/steamscreenshots/` directory, which the host launcher merges
- Tests: `tests/`, built and run through CTest with `BUILD_TESTING=ON` (needs an OpenGL 3.3 driver)
- Docs: `README.md` / `README.zh-CN.md`, prose pages under `docs/en/` and `docs/zh-CN/`,
  dependency notices in `docs/DEPENDENCIES.md`
- External sources, all read-only inputs: `METAHOOK_SOURCE_PATH` (public API and HLSDK),
  `STEAMSDK_SOURCE_PATH` (`steam_api.h` and `lib/steam_api.lib`), `GLEW_SOURCE_PATH` (configured as
  the static `libglew_static` target), `CAPSTONE_INCLUDE_DIRS` (tests), `GLFW_SOURCE_PATH` (tests
  only), `VC_LTL_Root`. Empty paths fall back to pinned FetchContent; VC-LTL is downloaded into
  `thirdparty/cache` and checked against its SHA256.
- Build output: `build/x86/<configuration>/`; install output: `install/x86/<configuration>/`. Neither
  is tracked, and nothing is deployed to the game automatically.

#### Progressive disclosure key points

- Read notes first, then locate a single file/symbol; do not read the whole repository at once.
- Prefer correctly scoped Basic Memory MCP tools for knowledge retrieval; otherwise use the local
  notes before reading source.
- Prefer Context7 for external dependency/library usage (query on demand).

## Repository rules

- Preserve the MetaHook API, plugin exports, calling conventions and capture behavior. Match the
  naming, indentation and comment style of the files you touch.
- Resolve engine private symbols only through the host gamedata contract
  (`ResolveGameSymbol`). A missing or failed symbol must keep the engine's original `snapshot`
  command, not add scan fallbacks; `SteamScreenshots` supports engines by availability of a
  presentation hook, not by hard-coded buildnum checks.
- Do not modify external sources or third-party sources. MetaHook, SteamSDK, GLEW, Capstone and GLFW
  are read-only build inputs; only GLEW is configured and built as a static library.
- The plugin builds for MSVC x86 only. Keep the static CRT / VC-LTL and C++20 settings in
  `CMakeLists.txt` in sync with the other standalone plugin repositories.
- When gamedata usage changes, update `scripts/manifests/steamscreenshots.json` in the same change.
- Regression tests keep assertions enabled even in Release; documentation and configuration text are
  not assertion targets.
- Verification distinguishes build/simulated tests from a real game run: the capture tests require an
  OpenGL 3.3 driver, and engine hook ordering plus Steam submission can only be verified in game.
  Claims about game or Steam behavior must not be made without evidence. Documentation changes need
  content, path and format checks, not a plugin rebuild.

## Explore SKILLs

- Project-level skills, when present, live in `.claude/skills` no matter what harness tool is being
  used.
