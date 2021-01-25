cmake_minimum_required(VERSION 3.11)

find_package(Python3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)
include_directories(SYSTEM ${PYTHON_INCLUDE_DIRS})

execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -m site --user-site
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE)



add_subdirectory(pypopminer)
add_subdirectory(pypoptesting)


add_custom_target(install_pypoptools
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/setup.py install --user
        )
function(pytest _name _py)

    add_custom_target(pypopminer_${_name}
            COMMAND ${PYTHON_EXECUTABLE} ${_py}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS pypopminer
            )
    add_dependencies(pypopminer_${_name} pypopminer install_pypoptools)
    add_test(
            NAME pypopminer_${_name}
            COMMAND ${PYTHON_EXECUTABLE} ${_py}
            WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    )
endfunction()

if(TESTING)


    pytest(mock_miner_test ${CMAKE_CURRENT_LIST_DIR}/tests/mock_miner_test.py)
    pytest(test_imports ${CMAKE_CURRENT_LIST_DIR}/tests/test_imports.py)
endif()