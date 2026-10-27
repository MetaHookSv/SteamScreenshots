# Dependency versions and notices

Plugin sources and capture tests originate from
[MetaHookSv](https://github.com/MetaHookSv/MetaHookSv) commit
`fe80b6d60bfb487b52aed7ea7ec0492e7b27a5d2`. The plugin source is copied unchanged;
the capture test's include path is adjusted for `src/`. Build helpers and the
gamedata synchronizer/validator follow the standalone ResourceReplacer repository.

| Dependency | Source | Revision |
| --- | --- | --- |
| MetaHook SDK | https://github.com/MetaHookSv/MetaHook | latest `main` |
| SteamSDK | https://github.com/MetaHookSv/SteamSDK | `3c1abaf6277f9f99fd16ef40557d6852820b848f` |
| GLEW | https://github.com/hzqst/glew-cmake | `459d9b95457c5db337e2742a5501c21faddda326` |
| Capstone headers | https://github.com/hzqst/capstone | `9e27c51c2485dd37dd3917919ae781c6153f3688` |
| GLFW (tests only) | https://github.com/MetaHookSv/glfw | `92dcf4ce74f2e2554a98fea09be7c705c17daa5a` |

Source checkouts reside in the build directory. GLFW is fetched only when tests are enabled.

The plugin's MIT license does not replace dependency licenses or applicable
terms. The local installation includes MetaHook's LICENSE, GLEW's LICENSE.txt and
SteamSDK's STEAM-SDK-NOTICE.md in their named subdirectories
under `svencoop/metahook/licenses/SteamScreenshots`. The 7z archive excludes
the `licenses/` directory.

SteamSDK retains Valve's copyright notices and applicable Steamworks terms.
It supplies the import library for this build. The release package does not
redistribute or overwrite `steam_api.dll`; the host game provides the runtime
and manages Steam initialization and callbacks.

Capstone is used for MetaHook API header types and is not linked. GLFW is used
only by capture tests and is not shipped. Neither library's headers or binaries
are included in the plugin archive.

The pruned gamedata comes from
[GoldSrc_VibeSignatures](https://github.com/hlnd2t/GoldSrc_VibeSignatures), through
its published gamesymbols catalog. Each synchronized snapshot is verified against
the catalog hash and validated against the SteamScreenshots consumer manifest.
