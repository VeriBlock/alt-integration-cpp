add_subdirectory(pypopminer)
add_subdirectory(pypoptesting)


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

if(TESTING)
    pytest(mock_miner_test tests/mock_miner_test.py)
    pytest(test_imports tests/test_imports.py)
endif()