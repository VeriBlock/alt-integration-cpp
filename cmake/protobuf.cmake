cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)
set(FETCHCONTENT_QUIET OFF)

if(MSVC OR COVERAGE)
    set(protobuf_BUILD_SHARED_LIBS ON)
endif()

set(protobuf_WITH_ZLIB_DEFAULT  OFF)
set(protobuf_WITH_ZLIB OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_TESTS OFF)
set(protobuf_BUILD_LIBPROTOC OFF)
set(protobuf_BUILD_CONFORMANCE OFF)
set(protobuf_BUILD_EXAMPLES OFF)
if(CMAKE_CROSSCOMPILING)
    # when cross-compiling do not build protoc
    set(protobuf_BUILD_PROTOC_BINARIES OFF)
else()
    set(protobuf_BUILD_PROTOC_BINARIES ON)
endif()
set(protobuf_DISABLE_RTTI ON)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "" FORCE)
set(protobuf_DEBUG_POSTFIX "" CACHE STRING "" FORCE)

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/google/protobuf.git
    GIT_TAG        v3.19.3
    GIT_PROGRESS   FALSE
    GIT_SHALLOW    TRUE
    USES_TERMINAL_DOWNLOAD TRUE
    GIT_SUBMODULES_RECURSE FALSE
    GIT_SUBMODULES ""
)

FetchContent_GetProperties(protobuf)
if(NOT protobuf_POPULATED)
    FetchContent_Populate(protobuf)
    add_subdirectory(${protobuf_SOURCE_DIR}/cmake ${protobuf_BINARY_DIR})
    set(_PROTOBUF_WELLKNOWN_INCLUDE_DIR "${protobuf_SOURCE_DIR}/src")
endif()


if(CMAKE_CROSSCOMPILING)
    find_program(_PROTOBUF_PROTOC_EXECUTABLE protoc REQUIRED)
else()
    if(NOT TARGET protoc)
        message(FATAL_ERROR "Can not find target protoc")
    endif()
    set(_PROTOBUF_PROTOC_EXECUTABLE $<TARGET_FILE:protoc>)
endif()


set(_PROTO_GENS_DIR ${CMAKE_BINARY_DIR}/gens)
file(MAKE_DIRECTORY ${_PROTO_GENS_DIR})


# Example:
# In CMake:
#     add_library(tezt tezt.cpp)
#     target_add_protobuf(tezt my.proto)
# Then, in the source code:
#     #include <my.pb.h>
function(target_add_protobuf target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target ${target} doesn't exist")
    endif()
    if(NOT ARGN)
        message(SEND_ERROR "Error: target_add_protobuf() called without any proto files")
        return()
    endif()

    set(_protobuf_include_path -I . -I ${_PROTOBUF_WELLKNOWN_INCLUDE_DIR})
    foreach(FIL ${ARGN})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)
        file(RELATIVE_PATH REL_FIL ${CMAKE_CURRENT_SOURCE_DIR} ${ABS_FIL})
        get_filename_component(REL_DIR ${REL_FIL} DIRECTORY)
        if(NOT REL_DIR)
            set(RELFIL_WE "${FIL_WE}")
        else()
            set(RELFIL_WE "${REL_DIR}/${FIL_WE}")
        endif()

        add_custom_command(
        OUTPUT  "${_PROTO_GENS_DIR}/${RELFIL_WE}.pb.cc"
                "${_PROTO_GENS_DIR}/${RELFIL_WE}.pb.h"
        COMMAND ${_PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --cpp_out=${_PROTO_GENS_DIR}
            ${_protobuf_include_path}
            ${REL_FIL}
        DEPENDS ${ABS_FIL} ${_PROTOBUF_PROTOC}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running C++ protocol buffer compiler on ${FIL}"
        VERBATIM)

        target_sources(${target} PRIVATE
            "${_PROTO_GENS_DIR}/${RELFIL_WE}.pb.cc"
            "${_PROTO_GENS_DIR}/${RELFIL_WE}.pb.h"
        )
        set(_proto_lib protobuf::libprotobuf)
        if (NOT TARGET ${_proto_lib})
            message(FATAL_ERROR "Expected target ${_proto_lib} to be defined")
        endif()
        target_link_libraries(${target} PRIVATE
            ${_proto_lib}
        )
        target_include_directories(${target} PRIVATE SYSTEM
            $<BUILD_INTERFACE:${_PROTO_GENS_DIR}>
            $<BUILD_INTERFACE:${_PROTOBUF_WELLKNOWN_INCLUDE_DIR}>
        )
        # disable -Werror for targets which contain protobuf
        if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(AppleClang|Clang|GNU)$")
            target_compile_options(${target} PRIVATE
                -Wno-error
            )
        # elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        #     target_compile_options(${target} PRIVATE
        #         /WX:NO
        #     )
        endif()
    endforeach()
endfunction()
