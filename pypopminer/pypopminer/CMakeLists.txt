set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

add_library(_pypopminer STATIC
        miner.cpp
        )

configure_file(__init__.py ${CMAKE_CURRENT_BINARY_DIR}/__init__.py COPYONLY)

target_link_libraries(_pypopminer
        ${Boost_LIBRARIES}
        ${PYTHON_LIBRARIES}
        ${LIB_NAME}
        )
set_target_properties(_pypopminer PROPERTIES
        PREFIX ""
        )
add_dependencies(pypopminer _pypopminer)