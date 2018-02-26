#################################################################################################################
# CMAKE SETTINGS
#################################################################################################################

# Additional user defined macros.
include(${CMAKE_SOURCE_DIR}/tools/cmake/macros.cmake)

#################################################################################################################
# PROJECT SETTINGS
#################################################################################################################

set(PROJECT_NAME "zone_allocator")
project(${PROJECT_NAME} CXX)

# Project paths.
set(ALLOCATOR_SRC_PATH ${CMAKE_SOURCE_DIR}/src)

#################################################################################################################
# TOOLCHAIN SETTINGS
#################################################################################################################

set(CMAKE_BUILD_TYPE Debug)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# List project configuration.
show_build_configuration()

#################################################################################################################
# OTHER SETTINGS
#################################################################################################################

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#################################################################################################################
# UNIT TEST APP
#################################################################################################################

include_directories(.)
include_directories(../src)

set(SOURCES
    catch.hpp
    main.cpp
    test_page.cpp
    test_page_allocator.cpp
    test_region_info.cpp
    utils.cpp
)

register_executable(${PROJECT_NAME})

link_targetEx(${PROJECT_NAME} ${ALLOCATOR_SRC_PATH})
