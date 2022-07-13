add_executable(liballocator-demo
    appMain.cpp
)

target_link_libraries(liballocator-demo
    PRIVATE liballocator platform::init platform::main
)

if (PLATFORM STREQUAL freertos-arm OR PLATFORM STREQUAL baremetal-arm)
    objcopy_generate_bin(liballocator-demo)
endif ()
