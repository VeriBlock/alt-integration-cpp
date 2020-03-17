cmake_minimum_required( VERSION 3.0.0 )

include( CMakeParseArguments )

find_package(Git)
if(GIT_FOUND)
    # Git describe
    execute_process(
            COMMAND           "${GIT_EXECUTABLE}" describe --tags
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE   git_result
            OUTPUT_VARIABLE   _DESCRIBE
            ERROR_VARIABLE    git_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
    )

    # get last tag
    execute_process(
            COMMAND           "${GIT_EXECUTABLE}" describe --tags --abbrev=0
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE   git_result
            OUTPUT_VARIABLE   _TAG
            ERROR_VARIABLE    git_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
    )

    if(_TAG STREQUAL _DESCRIBE)
        set(git_at_a_tag ON)
    endif()

    # get branch
    execute_process(
            COMMAND           "${GIT_EXECUTABLE}" symbolic-ref --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE   git_result
            OUTPUT_VARIABLE   _BRANCH
            ERROR_VARIABLE    git_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
    )

    # sanitize
    string(REGEX REPLACE "[^a-zA-Z0-9.]" "." _BRANCH_CLEAN ${_BRANCH})
    string(REGEX REPLACE "[.]+" "." _BRANCH_CLEAN ${_BRANCH_CLEAN})

    if(NOT _TAG)
        # get commit sha
        execute_process(
                COMMAND           "${GIT_EXECUTABLE}" rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE   git_result
                OUTPUT_VARIABLE   _SHA
                ERROR_VARIABLE    git_error
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_STRIP_TRAILING_WHITESPACE
        )
        set(_VERSION "${_BRANCH_CLEAN}-${_SHA}")
    else()
        if(git_at_a_tag)
            set(_VERSION ${_TAG})
        else()
            set(_VERSION "${_DESCRIBE}-${_BRANCH_CLEAN}")
        endif()
    endif()

    if( _TAG MATCHES "^([0-9]+).([0-9]+).([0-9]+)" )
        set( MAJOR_VERSION "${CMAKE_MATCH_1}" )
        set( MINOR_VERSION "${CMAKE_MATCH_2}" )
        set( PATCH_VERSION "${CMAKE_MATCH_3}" )
    endif()

    message(STATUS "[git tag]     : ${_TAG}")
    message(STATUS "[git branch]  : ${_BRANCH}")
    message(STATUS "[git version] : ${_VERSION}")

    set(VERSION ${_VERSION})
endif()
