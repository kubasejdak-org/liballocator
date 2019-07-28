add_executable(tests EXCLUDE_FROM_ALL
    main.cpp
    integration/integration_page_allocator.cpp
    integration/integration_zone_allocator.cpp
    perf/perf_allocator.cpp
    unit/unit_allocator.cpp
    unit/unit_group.cpp
    unit/unit_list_node.cpp
    unit/unit_page.cpp
    unit/unit_page_allocator.cpp
    unit/unit_region_info.cpp
    unit/unit_utils.cpp
    unit/unit_zone.cpp
    unit/unit_zone_allocator.cpp
)

# Link with private implementation of library for testing.
get_target_property(ALLOCATOR_INCLUDES liballocator INCLUDE_DIRECTORIES)

target_include_directories(tests
    PRIVATE . ${ALLOCATOR_INCLUDES}
)

target_compile_features(tests
    PRIVATE cxx_std_17
)

target_link_libraries(tests
    PUBLIC catch2
    PRIVATE liballocator
)

toolchain_setTargetOutputPath(tests ${CMAKE_BINARY_DIR}/bin)

# Test definitions.
get_target_property(RUNTIME_OUTPUT tests RUNTIME_OUTPUT_DIRECTORY)
add_test(UnitTests ${RUNTIME_OUTPUT}/tests [unit] "")
add_test(IntegrationTests ${RUNTIME_OUTPUT}/tests [integration] "")
add_test(PerformanceTests ${RUNTIME_OUTPUT}/tests [perf] "")
