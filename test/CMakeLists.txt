if (PLATFORM STREQUAL freertos-arm OR PLATFORM STREQUAL baremetal-arm)
    find_library(Catch2 NAMES Catch2 Catch2d REQUIRED)
endif ()

add_subdirectory(init)
add_subdirectory(${APP})
