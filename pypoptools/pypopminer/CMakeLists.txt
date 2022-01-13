set(Boost_USE_DEBUG_PYTHON OFF)
set(Boost_NO_WARN_NEW_VERSIONS TRUE)
foreach(ver "" "3" "36" "37" "38" "39" "310")
    find_package(Boost COMPONENTS python${ver})
    if(${Boost_FOUND})
        message(STATUS "Found boost-python${ver}")
        break()
    endif()
endforeach()

if (NOT ${Boost_FOUND})
    find_package(Boost REQUIRED COMPONENTS python)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

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
    add_custom_command(TARGET pypopminer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pypopminer> ${PYPOPMINER_OUTPUT_DIR}
        )
endif()

configure_file(__init__.py ${CMAKE_CURRENT_BINARY_DIR}/__init__.py COPYONLY)
