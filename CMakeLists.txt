# Required cmake version.
cmake_minimum_required(VERSION 3.9)

# Required C++ version.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
  # On macOS -search_paths_first linker flag breaks the build.
  string(REPLACE "-Wl,-search_paths_first" "" CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS})
  string(REPLACE "-Wl,-search_paths_first" "" CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})

endif()

include_directories(include)

add_subdirectory(test)
add_subdirectory(src)
