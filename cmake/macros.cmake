# Requires: SOURCES
macro(register_executable EXE_NAME)
    message(STATUS "Creating executable [${EXE_NAME}]")

    add_executable(${EXE_NAME} ${SOURCES})
endmacro()

# Requires: COMPONENTS
macro(register_packet)
    set(CURRENT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    get_filename_component(PACKET ${CURRENT_PATH} NAME)
    message(STATUS "Creating packet [${PACKET}]")

    foreach(COMPONENT ${COMPONENTS})
        add_subdirectory(${COMPONENT})
    endforeach()
endmacro()

# Requires: component.cpp [SOURCES] [MODULES]
macro(register_component)
    set(CURRENT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    get_filename_component(COMPONENT ${CURRENT_PATH} NAME)
    message(STATUS "Creating component [${COMPONENT}]")

    add_library(${COMPONENT} ${COMPONENT}.cpp ${SOURCES})
    set_property(TARGET ${COMPONENT} PROPERTY FOLDER ${COMPONENT})
    
    foreach(MODULE ${MODULES})
        add_subdirectory(${MODULE})
        set_property(TARGET ${MODULE} PROPERTY FOLDER ${COMPONENT})

        target_link_libraries(${COMPONENT} ${MODULE})
    endforeach()
endmacro()

# Requires: SOURCES
macro(register_module)
    set(CURRENT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    get_filename_component(MODULE ${CURRENT_PATH} NAME)
    message(STATUS "Creating module [${MODULE}]")

    add_library(${MODULE} ${SOURCES})
endmacro()

macro(link_target TARGET_NAME)
    set(CURRENT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    get_filename_component(CURRENT_TARGET ${CURRENT_PATH} NAME)
    get_filename_component(LINKED_TARGET ${TARGET_NAME} NAME)

    target_link_libraries(${CURRENT_TARGET} ${LINKED_TARGET})
endmacro()

macro(link_targetEx CURRENT_TARGET LINKED_TARGET_PATH)
    get_filename_component(TARGET_NAME ${LINKED_TARGET_PATH} NAME)
    set(LINKED_TARGET ${TARGET_NAME})

    target_link_libraries(${CURRENT_TARGET} ${LINKED_TARGET})
endmacro()

macro(show_build_configuration)
    message(STATUS "Project configuration:")
    message(STATUS "  Build type        : ${CMAKE_BUILD_TYPE}")
    message(STATUS "  C compiler        : ${CMAKE_C_COMPILER}")
    message(STATUS "  C++ compiler      : ${CMAKE_CXX_COMPILER}")
    message(STATUS "  assembler         : ${CMAKE_ASM_COMPILER}")
    message(STATUS "  ranlib            : ${CMAKE_RANLIB}")
    message(STATUS "  ar                : ${CMAKE_AR}")
    message(STATUS "  CFLAGS            : ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
    message(STATUS "  CXXFLAGS          : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
    message(STATUS "  C LDFLAGS         : ${CMAKE_C_LINK_FLAGS}")
    message(STATUS "  C++ LDFLAGS       : ${CMAKE_CXX_LINK_FLAGS}")
    message(STATUS "  C++ LDFLAGS (exe) : ${CMAKE_EXE_LINKER_FLAGS}")
endmacro()