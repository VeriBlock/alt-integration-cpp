# Download and unpack RocksDB at configure time
configure_file(CMakeLists-rocksdb.txt.in rocksdb-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rocksdb-download )
if(result)
    message(FATAL_ERROR "CMake step for RocksDB failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rocksdb-download )
if(result)
    message(FATAL_ERROR "Build step for RocksDB failed: ${result}")
endif()

set(WITH_GFLAGS 0 CACHE BOOL "build with gflags")
set(WITH_TESTS OFF CACHE BOOL "build with tests")
set(WITH_BENCHMARK_TOOLS OFF CACHE BOOL "build with benchmarks")
set(WITH_TOOLS OFF CACHE BOOL "build with tools")
set(FAIL_ON_WARNINGS OFF CACHE BOOL "fail on warnings")
set(ROCKSDB_BUILD_SHARED FALSE CACHE BOOL "build shared library alongside with static")
set(WITH_MD_LIBRARY OFF CACHE BOOL "build with MD runtime")
set(PORTABLE ON CACHE BOOL "build portable")
set(CMAKE_DISABLE_FIND_PACKAGE_NUMA TRUE CACHE BOOL "disable find NUMA package")
set(CMAKE_CXX_CLANG_TIDY "")

# Add RocksDB directly to our build.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/rocksdb-src
        ${CMAKE_CURRENT_BINARY_DIR}/rocksdb-build
        EXCLUDE_FROM_ALL)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(AppleClang|Clang|GNU)$")
    target_compile_options(rocksdb PUBLIC
        -Wno-format-y2k
        -Wno-double-promotion
        -Wno-format-nonliteral
        -Wno-unused-const-variable
        -Wno-maybe-uninitialized
        -Wno-unused-parameter)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(rocksdb PUBLIC
        /wd4242 # disable warning: '=': conversion from 'uint32_t' to 'unsigned char', possible loss of data
    )
endif()