set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(_pypopminer SHARED
        miner.cpp
        )
target_link_libraries(_pypopminer
        Boost::boost
        Boost::python
        Python3::Python
        ${LIB_NAME}
        )
set_target_properties(_pypopminer PROPERTIES
        PREFIX ""
        )
add_dependencies(pypopminer _pypopminer)