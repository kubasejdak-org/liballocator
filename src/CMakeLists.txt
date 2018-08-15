utils_log("Configuring '${CMAKE_CURRENT_SOURCE_DIR}/version.h'")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

add_library(allocator
    allocator.cpp
    chunk.cpp
    page_allocator.cpp
    page.cpp
    region_info.cpp
    zone.cpp
    zone_allocator.cpp
)

target_compile_features(allocator PUBLIC cxx_std_17)

target_include_directories(allocator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(allocator PUBLIC liballocator_include)

utils_setOutputPath(allocator ../bin)
utils_showTargetConfig(allocator)
