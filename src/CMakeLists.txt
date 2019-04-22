utils_log("Configuring '${CMAKE_CURRENT_SOURCE_DIR}/version.hpp'")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)

add_library(liballocator
    allocator.cpp
    group.cpp
    page_allocator.cpp
    page.cpp
    region_info.cpp
    zone.cpp
    zone_allocator.cpp
)

target_compile_features(liballocator
    PRIVATE cxx_std_17
)

target_include_directories(liballocator
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(liballocator
    PUBLIC liballocator_include
)

toolchain_setTargetOutputPath(liballocator ${PROJECT_SOURCE_DIR}/bin)
toolchain_showTargetConfig(liballocator)
