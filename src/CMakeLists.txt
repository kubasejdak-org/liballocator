utils_getVersion(VERSION)
project(liballocator VERSION ${VERSION} LANGUAGES CXX)

utils_log("Configuring '${CMAKE_CURRENT_SOURCE_DIR}/version.h'")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

add_library(allocator
    allocator.cpp
    page_allocator.cpp
    page.cpp
    region_info.cpp
    version.h
    zone_allocator.cpp
)

target_compile_features(allocator PUBLIC cxx_std_17)

target_include_directories(allocator
    PUBLIC ../include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
