cmake_minimum_required(VERSION 3.11)

find_package(Boost COMPONENTS python3)
if(NOT ${Boost_FOUND})
    find_package(Boost REQUIRED COMPONENTS python)
endif()
find_package(PythonInterp 3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)

add_subdirectory(src)

function(pytest _name _py)
    add_custom_target(pypopminer_${_name}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_py}
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
    pytest(mock_miner_test mock_miner_test.py)
endif()