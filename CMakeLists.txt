cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(SteamScreenshots LANGUAGES C CXX)

if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "SteamScreenshots requires Windows MSVC x86. Configure with -A Win32.")
endif()
get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(multi_config)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Supported configurations" FORCE)
elseif(NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release)$")
    message(FATAL_ERROR "Select Debug or Release.")
endif()
option(BUILD_TESTING "Build the OpenGL capture tests (requires an OpenGL 3.3 driver to run)" OFF)

include(cmake/Dependencies.cmake)
steamscreenshots_prepare_dependencies()
include("${VC_LTL_Root}/VC-LTL helper for cmake.cmake")

# Configure the existing GLEW fork without installing its SDK into the plugin package.
set(USE_VCLTL OFF CACHE BOOL "Separate vendor VC-LTL integration" FORCE)
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ONLY_LIBS ON CACHE BOOL "" FORCE)
# An aggregating parent build may already provide libglew_static from a sibling component.
if(NOT TARGET libglew_static)
    add_subdirectory("${GLEW_SOURCE_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/GLEW" EXCLUDE_FROM_ALL)
endif()
if(NOT TARGET libglew_static)
    message(FATAL_ERROR "GLEW_SOURCE_PATH must provide the libglew_static CMake target.")
endif()
# Group the vendored GLEW library under one IDE filter.
set_target_properties(libglew_static PROPERTIES FOLDER "glew")

# Include directories and definitions follow SteamScreenshots.vcxproj.
add_library(steamscreenshots_settings INTERFACE)
target_compile_features(steamscreenshots_settings INTERFACE cxx_std_20)
target_include_directories(steamscreenshots_settings INTERFACE
    src
    "${METAHOOK_SOURCE_PATH}/include"
    "${METAHOOK_SOURCE_PATH}/include/HLSDK/cl_dll"
    "${METAHOOK_SOURCE_PATH}/include/HLSDK/common"
    "${METAHOOK_SOURCE_PATH}/include/HLSDK/engine"
    "${METAHOOK_SOURCE_PATH}/include/HLSDK/pm_shared"
    "${METAHOOK_SOURCE_PATH}/include/HLSDK/public"
    "${METAHOOK_SOURCE_PATH}/include/Interface"
    "${GLEW_SOURCE_PATH}/include/GL"
    ${STEAMSCREENSHOTS_CAPSTONE_INCLUDE_DIRS}
)
target_compile_definitions(steamscreenshots_settings INTERFACE
    WIN32 _WINDOWS _USRDLL _MBCS GLEW_STATIC
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>)
target_compile_options(steamscreenshots_settings INTERFACE /permissive
    $<$<CONFIG:Debug>:/W0>
    $<$<CONFIG:Release>:/W3>
    /wd4311 /wd4312 /wd4819 /wd4996)

include(cmake/Sources.cmake)
add_library(SteamScreenshots SHARED ${STEAMSCREENSHOTS_SOURCES})
target_link_libraries(SteamScreenshots PRIVATE steamscreenshots_settings libglew_static SteamSDK::SteamAPI opengl32)
target_compile_options(SteamScreenshots PRIVATE /MP /Zi)
target_link_options(SteamScreenshots PRIVATE /DEBUG /SUBSYSTEM:WINDOWS
    $<$<CONFIG:Debug>:/INCREMENTAL>
    $<$<CONFIG:Release>:/INCREMENTAL:NO>
    $<$<CONFIG:Release>:/OPT:REF>
    $<$<CONFIG:Release>:/OPT:ICF>)
set_target_properties(SteamScreenshots PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

install(TARGETS SteamScreenshots RUNTIME DESTINATION svencoop/metahook/plugins)
install(FILES "$<TARGET_PDB_FILE:SteamScreenshots>" DESTINATION svencoop/metahook/plugins)

option(STEAMSCREENSHOTS_SYNC_GAMEDATA "Download and prune SteamScreenshots' gamedata catalog before building" ON)
set(STEAMSCREENSHOTS_GAMEDATA_DIR "${PROJECT_BINARY_DIR}/assets/svencoop/metahook/gamedata/steamscreenshots" CACHE PATH "SteamScreenshots gamedata catalog directory")
if(STEAMSCREENSHOTS_SYNC_GAMEDATA)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_custom_target(SteamScreenshotsGameData
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/sync-gamedata.py"
            --manifest "${CMAKE_CURRENT_SOURCE_DIR}/scripts/manifests/steamscreenshots.json"
            --target-dir "${STEAMSCREENSHOTS_GAMEDATA_DIR}" --temp-root "${PROJECT_BINARY_DIR}/gamedata-sync"
        COMMENT "Synchronizing and validating SteamScreenshots gamedata"
        VERBATIM
    )
    add_custom_target(SteamScreenshotsGameDataValidate
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/validate-gamedata.py"
            "${STEAMSCREENSHOTS_GAMEDATA_DIR}" --manifest "${CMAKE_CURRENT_SOURCE_DIR}/scripts/manifests/steamscreenshots.json"
        COMMENT "Validating SteamScreenshots gamedata against the manifest"
        VERBATIM
    )
    add_dependencies(SteamScreenshotsGameDataValidate SteamScreenshotsGameData)
    add_dependencies(SteamScreenshots SteamScreenshotsGameDataValidate)
endif()
install(DIRECTORY "${STEAMSCREENSHOTS_GAMEDATA_DIR}/" DESTINATION svencoop/metahook/gamedata/steamscreenshots OPTIONAL)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
