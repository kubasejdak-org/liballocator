set(SOURCES
    ${CMAKE_SOURCE_DIR}/include/zone_allocator/allocator.h
    ${CMAKE_SOURCE_DIR}/include/zone_allocator/region.h
    allocator.cpp
    page_allocator.cpp
    zone_allocator.cpp
)

register_module()
