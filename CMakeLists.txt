# Required cmake version.
cmake_minimum_required(VERSION 3.9)

# Required C++ version.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/usr/local/opt/llvm/include")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/opt/llvm/include")
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -L/usr/local/opt/llvm/lib")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -L/usr/local/opt/llvm/lib")
endif()

include_directories(include)

add_subdirectory(test)
add_subdirectory(src)
