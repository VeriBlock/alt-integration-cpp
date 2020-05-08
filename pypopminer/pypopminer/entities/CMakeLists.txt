set(CMAKE_LIBRARY_OUTPUT_DIRECTORY .)

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
        LIBRARY_OUTPUT_NAME _entities
        SUFFIX ".pyd"
        PREFIX ""
        )