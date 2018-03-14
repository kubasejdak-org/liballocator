cmake_minimum_required(VERSION 3.9)
include(tools/cmake/utils.cmake)

utils_getVersion(VERSION)
project(liballocator VERSION ${VERSION} LANGUAGES CXX)

# Project options.
option(WITH_TESTS "Build unit tests" OFF)

utils_setOutputPath(${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_subdirectory(src)
if(WITH_TESTS)
    utils_log("Building liballocator with tests")
    enable_testing()
    add_subdirectory(lib/catch2)
    add_subdirectory(test)
endif()
