# Download and unpack rocksdb at configure time
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
set(ROCKSDB_BUILD_SHARED FALSE CACHE BOOL "build shared library alongside with static")
set(WITH_BENCHMARK_TOOLS OFF CACHE BOOL "build with benchmarks")
set(WITH_TOOLS OFF CACHE BOOL "build with tools")
set(WITH_MD_LIBRARY OFF CACHE BOOL "build with MD runtime")

# Add RocksDB directly to our build. This defines the rocksdb target.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/rocksdb-src
        ${CMAKE_CURRENT_BINARY_DIR}/rocksdb-build
        EXCLUDE_FROM_ALL)