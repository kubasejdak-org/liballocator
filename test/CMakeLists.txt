add_executable(liballocator_tests
    main.cpp
    test_allocator.cpp
    test_list_node.cpp
    test_page.cpp
    test_page_allocator.cpp
    test_region_info.cpp
    test_utils.cpp
    test_zone.cpp
    test_zone_allocator.cpp
)

target_compile_features(liballocator_tests PRIVATE cxx_std_17)

# Link with library private implementation for testing.
get_target_property(ALLOCATOR_INCLUDES allocator INCLUDE_DIRECTORIES)
target_include_directories(liballocator_tests PRIVATE ${ALLOCATOR_INCLUDES})

target_link_libraries(liballocator_tests
    PUBLIC catch2 liballocator_include
    PRIVATE allocator
)

utils_setOutputPath(liballocator_tests ${CMAKE_SOURCE_DIR}/bin)

# Test definitions.
get_target_property(RUNTIME_OUTPUT liballocator_tests RUNTIME_OUTPUT_DIRECTORY)
add_test(UnitTests ${RUNTIME_OUTPUT}/liballocator_tests [unit] -r xml -d yes --order lex "")
add_test(IntegrationTests ${RUNTIME_OUTPUT}/liballocator_tests [integration] -r xml -d yes --order lex "")
add_test(PerformanceTests ${RUNTIME_OUTPUT}/liballocator_tests [perf] -r xml -d yes --order lex "")
