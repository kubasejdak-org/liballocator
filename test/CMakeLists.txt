add_executable(liballocator_tests
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

target_compile_features(liballocator_tests PRIVATE cxx_std_17)
target_include_directories(liballocator_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link with library private implementation for testing.
get_target_property(ALLOCATOR_INCLUDES liballocator INCLUDE_DIRECTORIES)
target_include_directories(liballocator_tests PRIVATE ${ALLOCATOR_INCLUDES})

target_link_libraries(liballocator_tests
    PUBLIC catch2 liballocator_include
    PRIVATE liballocator
)

toolchain_setTargetOutputPath(liballocator_tests ${CMAKE_SOURCE_DIR}/bin)

# Test definitions.
get_target_property(RUNTIME_OUTPUT liballocator_tests RUNTIME_OUTPUT_DIRECTORY)
add_test(UnitTests ${RUNTIME_OUTPUT}/liballocator_tests [unit] -r xml -d yes --order lex "")
add_test(IntegrationTests ${RUNTIME_OUTPUT}/liballocator_tests [integration] -r xml -d yes --order lex "")
add_test(PerformanceTests ${RUNTIME_OUTPUT}/liballocator_tests [perf] -r xml -d yes --order lex "")
