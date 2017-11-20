#################################################################################################################
# CMAKE SETTINGS
#################################################################################################################

# Additional user defined macros.
include(${CMAKE_SOURCE_DIR}/cmake/macros.cmake)

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

# Set common build options.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# List project configuration.
message(STATUS "Project configuration:")
message(STATUS "  Build type   : " ${CMAKE_BUILD_TYPE})

#################################################################################################################
# OTHER SETTINGS
#################################################################################################################

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#################################################################################################################
# UNIT TEST APP
#################################################################################################################

include_directories(.)

set(SOURCES
    catch.hpp
    main.cpp
    test_page_allocator.cpp
)

register_executable(${PROJECT_NAME})

link_targetEx(${PROJECT_NAME} ${ALLOCATOR_SRC_PATH})
