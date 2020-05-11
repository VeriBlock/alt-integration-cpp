set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

add_library(pyPopMiner SHARED
        miner.cpp
        )

target_link_libraries(pyPopMiner
        ${Boost_LIBRARIES}
        ${PYTHON_LIBRARIES}
        ${LIB_NAME}
        )

set_target_properties(pyPopMiner PROPERTIES
        PREFIX ""
        )

configure_file(__init__.py ${CMAKE_CURRENT_BINARY_DIR}/__init__.py COPYONLY)

execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -m site --user-site
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

install(TARGETS pyPopMiner DESTINATION "${PYTHON_SITE}/pyPopMiner")
install(FILES __init__.py DESTINATION "${PYTHON_SITE}/pyPopMiner")