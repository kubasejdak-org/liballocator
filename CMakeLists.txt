# Required cmake version.
cmake_minimum_required(VERSION 3.10)

# Required C++ version.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_subdirectory(test)
add_subdirectory(src)
