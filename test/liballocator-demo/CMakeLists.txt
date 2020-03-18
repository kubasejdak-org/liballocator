add_library(appliballocator-demo
    appMain.cpp
)

target_compile_features(appliballocator-demo
    PRIVATE cxx_std_17
)

target_link_libraries(appliballocator-tests
    PRIVATE platform-init liballocator
)
