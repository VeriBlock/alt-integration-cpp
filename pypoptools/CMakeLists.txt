cmake_minimum_required(VERSION 3.11)

find_package(PythonInterp 3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)
include_directories(SYSTEM ${PYTHON_INCLUDE_DIRS})

execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -m site --user-site
        OUTPUT_VARIABLE PYTHON_SITE
        OUTPUT_STRIP_TRAILING_WHITESPACE)

add_subdirectory(pypopminer)
add_subdirectory(pypoptesting)
