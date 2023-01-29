add_library(platform-init EXCLUDE_FROM_ALL)
add_library(platform::init ALIAS platform-init)

target_include_directories(platform-init
    PUBLIC include
)

add_subdirectory(${PLATFORM})

if (PLATFORM STREQUAL freertos-arm OR PLATFORM STREQUAL baremetal-arm)
    target_link_libraries(platform-init
        INTERFACE optimized Catch2 debug Catch2d
    )
endif ()
