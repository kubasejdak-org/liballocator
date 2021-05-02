add_library(appliballocator-tests OBJECT EXCLUDE_FROM_ALL
    appMain.cpp
    integration/PageAllocator.cpp
    integration/ZoneAllocator.cpp
    perf/allocator.cpp
    unit/allocator.cpp
    unit/group.cpp
    unit/ListNode.cpp
    unit/Page.cpp
    unit/PageAllocator.cpp
    unit/RegionInfo.cpp
    unit/utils.cpp
    unit/Zone.cpp
    unit/ZoneAllocator.cpp
)

# Link with private implementation of library for testing.
get_target_property(ALLOCATOR_INCLUDES liballocator INCLUDE_DIRECTORIES)

target_include_directories(appliballocator-tests
    PRIVATE . ${ALLOCATOR_INCLUDES}
)

conan_get(REQUIRES catch2/2.13.6)
find_package(Catch2 REQUIRED)

target_compile_definitions(appliballocator-tests
    # This is required to selectively run Catch2 tests using tags on platforms, where cmd line arguments are not available.
    PRIVATE $<$<BOOL:${TEST_TAGS}>:TEST_TAGS="${TEST_TAGS}">
)

target_compile_definitions(Catch2::Catch2
    INTERFACE CATCH_CONFIG_DISABLE_EXCEPTIONS $<$<STREQUAL:${PLATFORM},freertos-arm>:CATCH_CONFIG_NO_POSIX_SIGNALS>
)

target_link_libraries(appliballocator-tests
    PRIVATE platform-init Catch2::Catch2 liballocator
)
