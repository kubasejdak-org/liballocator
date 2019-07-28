cmake_minimum_required(VERSION 3.14)
include(tools/cmake/all.cmake)

utils_getVersion(VERSION)
project(liballocator VERSION ${VERSION} LANGUAGES CXX)

# Project options.
option(LIBALLOCATOR_TESTS "Build unit tests" ON)
option(LIBALLOCATOR_DOCS "Generate doxygen documentation" ON)
option(LIBALLOCATOR_COVERAGE "Generate coverage report" OFF)

# Common CMake options.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Project-wide compilation options.
add_compile_options(-Wall -Wextra -Werror)

if(LIBALLOCATOR_COVERAGE)
    utils_log("Building liballocator with coverage report")
    set(COVERAGE_IGNORE '${PROJECT_SOURCE_DIR}/test/*' '${PROJECT_SOURCE_DIR}/lib/*')
    coverage_addTarget(${COVERAGE_IGNORE})
endif()

add_subdirectory(lib)
add_subdirectory(src)

if(LIBALLOCATOR_TESTS)
    toolchain_showConfig()
    utils_log("Building liballocator with tests")
    enable_testing()
    add_subdirectory(test)
endif()

if(LIBALLOCATOR_DOCS)
    utils_log("Building liballocator with docs")
    doxygen_addTarget(${PROJECT_SOURCE_DIR})
endif()

# Export targets.
export(TARGETS liballocator NAMESPACE liballocator FILE liballocator.cmake)
