# Explicit compile list from MetaHookSv fe80b6d SteamScreenshots.vcxproj.
# The SDK supplies CreateInterface and the ServerName message parser.
set(STEAMSCREENSHOTS_SOURCES
    "${METAHOOK_SOURCE_PATH}/include/HLSDK/common/interface.cpp"
    "${METAHOOK_SOURCE_PATH}/include/HLSDK/common/parsemsg.cpp"
    "${PROJECT_SOURCE_DIR}/src/exportfuncs.cpp"
    "${PROJECT_SOURCE_DIR}/src/plugins.cpp"
    "${PROJECT_SOURCE_DIR}/src/gl_capture.cpp"
)
