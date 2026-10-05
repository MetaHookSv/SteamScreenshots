# GLFW is shared through GLFW_SOURCE_PATH (the aggregator injects the
# repository-level thirdparty/glfw submodule; a standalone build resolves it in
# cmake/Dependencies.cmake) and is configured once per solution.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)
# A sibling component's tests, or Renderer's, may have added the shared target.
if(NOT TARGET glfw)
    add_subdirectory("${GLFW_SOURCE_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/glfw" EXCLUDE_FROM_ALL)
endif()

add_executable(capture_tests capture_tests.cpp ../src/gl_capture.cpp)
target_compile_features(capture_tests PRIVATE cxx_std_20)
set_target_properties(capture_tests PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(capture_tests PRIVATE GLEW_STATIC GLFW_INCLUDE_NONE)
target_include_directories(capture_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/support"
    # capture_tests.cpp includes the flat <glew.h>, which the glfw3.h chain does
    # not provide; libglew_static only exports <GL/glew.h>.
    "${GLEW_SOURCE_PATH}/include/GL")
# GLFW_INCLUDE_NONE keeps glfw3.h from pulling in a conflicting system GL header.
target_link_libraries(capture_tests PRIVATE glfw libglew_static)

foreach(test_case IN ITEMS sync-default async-default sync-dirty async-dirty
        sync-pending async-frame-pending wait-timeout wait-failure
        framebuffer-unavailable framebuffer-entrypoint-unavailable arb-framebuffer)
    add_test(NAME "capture.${test_case}" COMMAND capture_tests "${test_case}")
    set_tests_properties("capture.${test_case}" PROPERTIES TIMEOUT 20)
endforeach()
