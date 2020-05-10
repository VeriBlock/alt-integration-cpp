set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(_entities SHARED
        entities.cpp
        )
target_link_libraries(_entities
        Boost::boost
        Boost::python
        Python3::Python
        ${LIB_NAME}
        )
set_target_properties(_entities PROPERTIES
        PREFIX ""
        )