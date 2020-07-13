add_library(appliballocator-demo
    appMain.cpp
)

target_link_libraries(appliballocator-demo
    PUBLIC platform-init
    PRIVATE liballocator
)
