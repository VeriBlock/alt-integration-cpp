# use bundled rocksdb
set(WITH_GFLAGS 0 CACHE BOOL "build with gflags")
set(WITH_TESTS OFF CACHE BOOL "build with tests")
set(WITH_BENCHMARK_TOOLS OFF CACHE BOOL "build with benchmarks")
set(WITH_TOOLS OFF CACHE BOOL "build with tools")
set(WITH_EXAMPLES OFF CACHE BOOL "build examples")
set(FAIL_ON_WARNINGS OFF CACHE BOOL "fail on warnings")
set(ROCKSDB_BUILD_SHARED FALSE CACHE BOOL "build shared library alongside with static")
set(WITH_MD_LIBRARY OFF CACHE BOOL "build with MD runtime")
set(PORTABLE ON CACHE BOOL "build portable")
set(CMAKE_DISABLE_FIND_PACKAGE_NUMA TRUE CACHE BOOL "disable find NUMA package")

FetchContent_Declare(
        rocksdb
        GIT_REPOSITORY https://github.com/facebook/rocksdb.git
        GIT_TAG        v6.19.3
)

function(disable_from_all target)
    set_target_properties(${target} PROPERTIES
            EXCLUDE_FROM_ALL TRUE
            )
endfunction()

FetchContent_GetProperties(rocksdb)
if(NOT rocksdb_POPULATED)
    message(STATUS "Downloading rocksdb...")
    FetchContent_Populate(rocksdb)
    add_subdirectory(${rocksdb_SOURCE_DIR} ${rocksdb_BINARY_DIR})
    target_include_directories(rocksdb PUBLIC
            $<BUILD_INTERFACE:${rocksdb_SOURCE_DIR}/include>
            )
    include_directories($<BUILD_INTERFACE:${rocksdb_SOURCE_DIR}/include>)
    disable_from_all(ldb)
    disable_clang_tidy(ldb)

    disable_from_all(sst_dump)
    disable_clang_tidy(sst_dump)
endif()


if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(AppleClang|Clang|GNU)$")
    target_add_flag(rocksdb -Wno-format)
    target_add_flag(rocksdb -Wno-null-dereference)
    target_add_flag(rocksdb -Wno-format-y2k)
    target_add_flag(rocksdb -Wno-format)
    target_add_flag(rocksdb -Wno-double-promotion)
    target_add_flag(rocksdb -Wno-format-nonliteral)
    target_add_flag(rocksdb -Wno-unused-const-variable)
    target_add_flag(rocksdb -Wno-maybe-uninitialized)
    target_add_flag(rocksdb -Wno-unused-parameter)
    target_add_flag(rocksdb -Wno-range-loop-construct)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # disable warning: '=': conversion from 'uint32_t' to 'unsigned char', possible loss of data
    target_add_flag(rocksdb /wd4242)
endif()

add_compile_definitions(WITH_ROCKSDB)