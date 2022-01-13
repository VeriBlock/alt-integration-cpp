find_package(Python3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)
include_directories(SYSTEM ${PYTHON_INCLUDE_DIRS})

if(NOT PYTHON_EXECUTABLE)
    find_program(PY3 python3)
    if(PY3)
        set(PYTHON_EXECUTABLE ${PY3})
        find_program(PYTHON_EXECUTABLE python REQUIRED)
    endif()
endif()

execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -m site --user-site
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

add_subdirectory(pypopminer)
add_subdirectory(pypoptesting)

add_custom_target(install_pypoptools
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/setup.py install --user
        DEPENDS ${LIB_NAME}
        COMMENT "Installing pypoptools..."
        )
