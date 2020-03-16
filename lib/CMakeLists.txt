message(STATUS "Configuring '${CMAKE_CURRENT_SOURCE_DIR}/version.hpp'")
configure_file(version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)

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

target_compile_features(liballocator
    PRIVATE cxx_std_17
)
