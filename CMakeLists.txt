cmake_minimum_required(VERSION 3.9)
include(tools/cmake/utils.cmake)

utils_getVersion(VERSION)
project(liballocator VERSION ${VERSION} LANGUAGES CXX)

# Project options.
option(LIBALLOCATOR_TESTS "Build unit tests" OFF)

# Common compilation flags.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Werror)
utils_showToolchainConfig()

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(src)

if(LIBALLOCATOR_TESTS)
    utils_log("Building liballocator with tests")
    enable_testing()
    add_subdirectory(test)
endif()
