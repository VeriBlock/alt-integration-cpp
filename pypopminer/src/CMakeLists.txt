set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${PYTHON_INCLUDE_DIRS})
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

configure_file(__init__.py ${CMAKE_CURRENT_BINARY_DIR}/__init__.py COPYONLY)

execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -m site --user-site
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

install(TARGETS pypopminer DESTINATION "${PYTHON_SITE}/pypopminer")
install(FILES __init__.py DESTINATION "${PYTHON_SITE}/pypopminer")