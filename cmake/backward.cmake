find_library(DW dw QUIET)
find_library(BFD bfd QUIET)
find_library(DWARF dwarf QUIET)
if (NOT DB AND NOT BFD AND NOT DWARF)
    message(STATUS "Adding stacktrace is not available because none of libdw/libbfd/libdwarf is installed.")
    function(enable_stacktrace_on_target target)
        # do nothing
    endfunction()
    return()
endif()

if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message(STATUS "Adding stacktrace is available only in CMAKE_BUILD_TYPE=Debug")
    function(enable_stacktrace_on_target target)
        # do nothing
    endfunction()
    return()
endif()

FetchContent_Declare(
        backward
        GIT_REPOSITORY https://github.com/bombela/backward-cpp.git
        GIT_TAG        5ffb2c879ebdbea3bdb8477c671e32b1c984beaa
        GIT_PROGRESS   TRUE
        GIT_SHALLOW    TRUE
        USES_TERMINAL_DOWNLOAD TRUE
        GIT_SUBMODULES_RECURSE FALSE
        GIT_SUBMODULES ""
)

set(BACKWARD_TESTS OFF)

FetchContent_GetProperties(backward)
if(NOT backward_POPULATED)
    message(STATUS "Downloading 'backward'...")
    FetchContent_Populate(backward)
    add_subdirectory(${backward_SOURCE_DIR} ${backward_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


function(enable_stacktrace_on_target target)
    target_sources(${target} PRIVATE ${BACKWARD_ENABLE})
    add_backward(${target})
endfunction()
