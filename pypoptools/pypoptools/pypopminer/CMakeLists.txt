find_package(Boost QUIET COMPONENTS python3)
if(NOT ${Boost_FOUND})
    find_package(Boost REQUIRED COMPONENTS python)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

function(pytest _name _py)
    add_custom_target(pypopminer_${_name}
            COMMAND ${PYTHON_EXECUTABLE} ${_py}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS pypopminer
            )
    add_dependencies(pypopminer_${_name} pypopminer)
    add_test(
            NAME pypopminer_${_name}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.py
            WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    )
endfunction()

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
        set_target_properties(pypopminer PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY ${PYPOPMINER_OUTPUT_DIR}
                )

endif()

configure_file(__init__.py ${CMAKE_CURRENT_BINARY_DIR}/__init__.py COPYONLY)

install(TARGETS pypopminer DESTINATION "${PYTHON_SITE}/pypopminer")
install(FILES __init__.py DESTINATION "${PYTHON_SITE}/pypopminer")


if(TESTING)
    pytest(mock_miner_test mock_miner_test.py)
endif()