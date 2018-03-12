cmake_minimum_required(VERSION 3.9)
include(tools/cmake/utils.cmake)

# Project options.
option(WITH_TESTS "Build unit tests" OFF)

utils_setOutputPath(${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(src)
if(WITH_TESTS)
    utils_log("Building liballocator with tests")
    enable_testing()
    add_subdirectory(lib/catch2)
    add_subdirectory(test)
endif()
