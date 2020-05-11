set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PYTHON_INCLUDE_DIRS})

add_library(_pypopminer SHARED
        miner.cpp
        )
target_link_libraries(_pypopminer
        Boost::boost
        Boost::python
        ${PYTHON_LIBRARIES}
        ${LIB_NAME}
        )
set_target_properties(_pypopminer PROPERTIES
        PREFIX ""
        )
add_dependencies(pypopminer _pypopminer)