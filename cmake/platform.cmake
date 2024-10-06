include(FetchContent)
FetchContent_Declare(platform
    GIT_REPOSITORY  https://github.com/kubasejdak-org/platform.git
    GIT_TAG         e28c06cd350da843db437ad023bdab8a4e00deed
)

FetchContent_GetProperties(platform)
if (NOT platform_POPULATED)
    FetchContent_Populate(platform)
endif ()

if (NOT DEFINED PLATFORM)
    set(DEFAULT_PLATFORM    linux)
    message(STATUS "'PLATFORM' is not defined. Using '${DEFAULT_PLATFORM}'")
    set(PLATFORM            ${DEFAULT_PLATFORM})
endif ()

# Setup platform toolchain file.
include(${platform_SOURCE_DIR}/lib/${PLATFORM}/toolchain.cmake)
