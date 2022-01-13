find_package(Boost QUIET COMPONENTS python3)
if(NOT ${Boost_FOUND})
    find_package(Boost QUIET COMPONENTS python)
    if (NOT ${Boost_FOUND})
        find_package(Boost MODULE REQUIRED COMPONENTS python)
    endif()
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_library(pypopminer MODULE
        miner.cpp
        entities.cpp
        primitives.cpp
        )

target_link_libraries(pypopminer
        ${Boost_LIBRARIES}
        ${PYTHON_LIBRARIES}
        ${LIB_NAME}
        )

set_target_properties(pypopminer PROPERTIES
        PREFIX ""
        )
if(PYPOPMINER_OUTPUT_DIR)
    add_custom_command(TARGET pypopminer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pypopminer> ${PYPOPMINER_OUTPUT_DIR}
        )
endif()

configure_file(__init__.py ${CMAKE_CURRENT_BINARY_DIR}/__init__.py COPYONLY)
