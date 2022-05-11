add_library(appliballocator-demo
    appMain2.cpp
)

target_link_libraries(appliballocator-demo
    PUBLIC platform-init
    #PRIVATE liballocator
)
