add_executable(tests
    main.cpp
    test_page_allocator.cpp
    test_page.cpp
    test_region_info.cpp
    utils.h
)

target_compile_features(tests PRIVATE cxx_std_17)

# Link with library private implementation for testing.
get_target_property(ALLOCATOR_INCLUDES allocator INCLUDE_DIRECTORIES)
target_include_directories(tests PRIVATE ${ALLOCATOR_INCLUDES})

target_link_libraries(tests
    PUBLIC catch2 liballocator_include
    PRIVATE allocator
)

utils_setOutputPath(tests ${CMAKE_SOURCE_DIR}/bin)

# Test definitions.
add_test(UnitTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests -r xml -d yes --order lex "")
add_test(IntegrationTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests [.][integration] -r xml -d yes --order lex "")

utils_showBuildConfig()
