cmake_minimum_required(VERSION 3.11)

set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost COMPONENTS python3)
if(NOT ${Boost_FOUND})
    find_package(Boost REQUIRED COMPONENTS python)
endif()
find_package(PythonInterp 3)
find_package(PythonLibs 3)
add_custom_target(src COMMENT "Building pyPopMiner")

add_subdirectory(src)

function(pytest _name _py)
    add_custom_target(pyPopMiner_${_name}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_py}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS pyPopMiner
            )
    add_dependencies(pyPopMiner_${_name} pyPopMiner)
    add_test(
            NAME pyPopMiner_${_name}
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_name}.py
            WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    )
endfunction()

if(TESTING)
    pytest(mock_miner_test mock_miner_test.py)
endif()