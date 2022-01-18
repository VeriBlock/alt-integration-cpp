cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)

set(FETCHCONTENT_QUIET OFF)
set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL ON)

find_package(ZLIB)
if(ZLIB_FOUND)
    set(_protobuf_ZLIB ON)
else()
    set(_protobuf_ZLIB OFF)
endif()
set(protobuf_WITH_ZLIB ${_protobuf_ZLIB} CACHE BOOL "" FORCE)
set(protobuf_WITH_ZLIB ${_protobuf_ZLIB})
set(protobuf_WITH_ZLIB_DEFAULT  ${_protobuf_ZLIB})


if(CMAKE_CROSSCOMPILING)
    message(STATUS "Architectures: ${CMAKE_OSX_ARCHITECTURES}")
    list(LENGTH CMAKE_OSX_ARCHITECTURES CMAKE_OSX_ARCHITECTURES_LENGTH)
    if(CMAKE_OSX_ARCHITECTURES_LENGTH EQUAL 0)
        set(OPENSSL_NO_ASM TRUE)
    endif()
endif()


set(protobuf_BUILD_TESTS OFF)
set(protobuf_BUILD_CONFORMANCE OFF)
set(protobuf_BUILD_EXAMPLES OFF)
set(protobuf_BUILD_PROTOC_BINARIES ON)
set(protobuf_DISABLE_RTTI ON)
set(protobuf_MSVC_STATIC_RUNTIME ON)
set(protobuf_DEBUG_POSTFIX "" CACHE STRING "" FORCE)
if(FIND_PROTOBUF)
    # search for protobuf in local OS
    find_package(Protobuf REQUIRED)

    set(gRPC_PROTOBUF_PROVIDER "package" CACHE STRING "" FORCE)

    if(TARGET protobuf::libprotoc)
      set(_gRPC_PROTOBUF_PROTOC_LIBRARIES protobuf::libprotoc)
      # extract the include dir from target's properties
      get_target_property(_gRPC_PROTOBUF_WELLKNOWN_INCLUDE_DIR protobuf::libprotoc INTERFACE_INCLUDE_DIRECTORIES)
    else()
      set(_gRPC_PROTOBUF_PROTOC_LIBRARIES ${PROTOBUF_PROTOC_LIBRARIES})
      set(_gRPC_PROTOBUF_WELLKNOWN_INCLUDE_DIR ${PROTOBUF_INCLUDE_DIRS})
    endif()
else()
    # download protobuf
    set(gRPC_PROTOBUF_PROVIDER "module" CACHE STRING "" FORCE)

    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/google/protobuf.git
        GIT_TAG        v3.19.3
        GIT_PROGRESS   TRUE
        GIT_SHALLOW    TRUE
        USES_TERMINAL_DOWNLOAD TRUE
        GIT_SUBMODULES_RECURSE FALSE
        GIT_SUBMODULES ""
    )

    FetchContent_GetProperties(protobuf)
    if(NOT protobuf_POPULATED)
        FetchContent_Populate(protobuf)
        # do not add_subdirectory into protobuf_SOURCE_DIR, because
        # grpc will do it anyway.
        set(PROTOBUF_ROOT_DIR "${protobuf_SOURCE_DIR}")
        set(_gRPC_PROTOBUF_WELLKNOWN_INCLUDE_DIR "${protobuf_SOURCE_DIR}/src")
    endif()
endif()


if(FIND_GRPC)
    find_package(grpc REQUIRED)
else()
    FetchContent_Declare(
            grpc
            GIT_REPOSITORY https://github.com/grpc/grpc.git
            GIT_TAG        v1.43.0
            GIT_PROGRESS   TRUE
            GIT_SHALLOW    TRUE
            USES_TERMINAL_DOWNLOAD TRUE
            GIT_SUBMODULES_RECURSE FALSE
            GIT_SUBMODULES
                "third_party/cares"
                "third_party/boringssl-with-bazel"
                "third_party/re2"
                "third_party/abseil-cpp"
    )

    set(gRPC_BUILD_TESTS OFF)
    set(gRPC_BUILD_CODEGEN ON) # for grpc_cpp_plugin
    set(gRPC_BUILD_GRPC_CPP_PLUGIN ON) # we want to use only C++ plugin
    set(gRPC_BUILD_CSHARP_EXT OFF)
    set(gRPC_BUILD_GRPC_CSHARP_PLUGIN OFF)
    set(gRPC_BUILD_GRPC_NODE_PLUGIN OFF)
    set(gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN OFF)
    set(gRPC_BUILD_GRPC_PHP_PLUGIN OFF)
    set(gRPC_BUILD_GRPC_PYTHON_PLUGIN OFF)
    set(gRPC_BUILD_GRPC_RUBY_PLUGIN OFF)

    set(gRPC_BENCHMARK_PROVIDER "none" CACHE STRING "" FORCE)
    set(gRPC_ZLIB_PROVIDER "package" CACHE STRING "" FORCE)

    # use lite protobuf version, unless we start using features
    # that require full protobuf
    set(gRPC_USE_PROTO_LITE ON CACHE BOOL "" FORCE)


    FetchContent_GetProperties(grpc)
    if(NOT grpc_POPULATED)
        FetchContent_Populate(grpc)
        add_subdirectory(${grpc_SOURCE_DIR} ${grpc_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()


if(CMAKE_CROSSCOMPILING)
    find_program(_gRPC_PROTOBUF_PROTOC_EXECUTABLE protoc REQUIRED)
else()
    set(_gRPC_PROTOBUF_PROTOC_EXECUTABLE $<TARGET_FILE:protoc>)
endif()


set(_gRPC_PROTO_GENS_DIR ${CMAKE_BINARY_DIR}/gens)
file(MAKE_DIRECTORY ${_gRPC_PROTO_GENS_DIR})


# Example:
# In CMake:
#     add_library(tezt tezt.cpp)
#     target_add_protobuf(tezt my.proto)
# Then, in the source code:
#     #include <my.pb.h>
#     #include <my.grpc.pb.h>
function(target_add_protobuf target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Target ${target} doesn't exist")
    endif()
    if(NOT ARGN)
        message(SEND_ERROR "Error: PROTOBUF_GENERATE_GRPC_CPP() called without any proto files")
        return()
    endif()

    set(_protobuf_include_path -I . -I ${_gRPC_PROTOBUF_WELLKNOWN_INCLUDE_DIR})
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

        if(CMAKE_CROSSCOMPILING)
            find_program(_gRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)
        else()
            set(_gRPC_CPP_PLUGIN $<TARGET_FILE:grpc_cpp_plugin>)
        endif()

        add_custom_command(
        OUTPUT  "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}.grpc.pb.cc"
                "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}.grpc.pb.h"
                "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}_mock.grpc.pb.h"
                "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}.pb.cc"
                "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}.pb.h"
        COMMAND ${_gRPC_PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --grpc_out=generate_mock_code=true:${_gRPC_PROTO_GENS_DIR}
            --cpp_out=${_gRPC_PROTO_GENS_DIR}
            --plugin=protoc-gen-grpc=${_gRPC_CPP_PLUGIN}
            ${_protobuf_include_path}
            ${REL_FIL}
        DEPENDS ${ABS_FIL} ${_gRPC_PROTOBUF_PROTOC} ${_gRPC_CPP_PLUGIN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running gRPC C++ protocol buffer compiler on ${FIL}"
        VERBATIM)

        target_sources(${target} PRIVATE
            "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}.grpc.pb.cc"
            "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}.grpc.pb.h"
            "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}_mock.grpc.pb.h"
            "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}.pb.cc"
            "${_gRPC_PROTO_GENS_DIR}/${RELFIL_WE}.pb.h"
        )
        target_include_directories(${target} PRIVATE SYSTEM
            $<BUILD_INTERFACE:${_gRPC_PROTO_GENS_DIR}>
            $<BUILD_INTERFACE:${_gRPC_PROTOBUF_WELLKNOWN_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${grpc_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${grpc_SOURCE_DIR}/third_party/abseil-cpp>
        )
        # disable -Werror for targets which contain protobuf
        if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(AppleClang|Clang|GNU)$")
            target_compile_options(${target} PRIVATE
                -Wno-error
            )
        elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            target_compile_options(${target} PRIVATE
                /WX:NO
            )
        endif()
    endforeach()
endfunction()

# override
set(CMAKE_DEBUG_POSTFIX "")
