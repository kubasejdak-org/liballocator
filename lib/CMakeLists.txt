cmake_minimum_required(VERSION 3.24)

message(STATUS "Configuring '${CMAKE_CURRENT_SOURCE_DIR}/version.hpp'")
configure_file(version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)

# Project-wide compilation options.
add_compile_options(-Wall -Wextra -Wpedantic -Werror $<$<COMPILE_LANGUAGE:CXX>:-std=c++20> $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)

add_library(liballocator
    allocator.cpp
    group.cpp
    Page.cpp
    PageAllocator.cpp
    RegionInfo.cpp
    Zone.cpp
    ZoneAllocator.cpp
)

target_include_directories(liballocator
    PUBLIC include
    PRIVATE .
)
