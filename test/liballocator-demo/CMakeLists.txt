add_library(appliballocator-demo EXCLUDE_FROM_ALL
    appMain.cpp
)

target_link_libraries(appliballocator-demo
    PUBLIC platform-init
    PRIVATE liballocator
)
