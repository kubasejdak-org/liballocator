utils_log("Configuring '${CMAKE_CURRENT_SOURCE_DIR}/version.h'")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/version.h)

add_library(liballocator
    allocator.cpp
    page_allocator.cpp
    page.cpp
    region_info.cpp
    zone.cpp
    zone_allocator.cpp
)

target_compile_features(liballocator PRIVATE cxx_std_17)

target_include_directories(liballocator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(liballocator PUBLIC liballocator_include)

toolchain_setTargetOutputPath(liballocator ${PROJECT_SOURCE_DIR}/bin)
toolchain_showTargetConfig(liballocator)
