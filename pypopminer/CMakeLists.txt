set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost REQUIRED COMPONENTS python)
find_package(Python3 REQUIRED COMPONENTS Development Interpreter)
add_custom_target(pypopminer COMMENT "Building pypopminer")

add_subdirectory(pypopminer)

function(pytest _name _py)
    add_custom_target(pypopminer_${_name}
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${_py}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
    add_dependencies(pypopminer_${_name} pypopminer)
    add_test(
            NAME pypopminer_${_name}
            COMMAND pypopminer_${_name}
    )
endfunction()

if(TESTING)
    pytest(mock_miner_test mock_miner_test.py)
endif()