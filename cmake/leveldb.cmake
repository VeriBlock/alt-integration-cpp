# use bundled leveldb
set(LEVELDB_BUILD_TESTS OFF CACHE BOOL "Build LevelDB's unit tests")
set(LEVELDB_BUILD_BENCHMARKS OFF CACHE BOOL "Build LevelDB's benchmarks")
set(HAVE_SNAPPY OFF CACHE BOOL "Build with snappy compression library")

FetchContent_Declare(
        leveldb
        GIT_REPOSITORY https://github.com/google/leveldb.git
        GIT_TAG        1.23
)

FetchContent_GetProperties(leveldb)
if(NOT leveldb_POPULATED)
    message(STATUS "Downloading leveldb...")
    FetchContent_Populate(leveldb)
    add_subdirectory(${leveldb_SOURCE_DIR} ${leveldb_BINARY_DIR})
    target_include_directories(leveldb PUBLIC
            $<BUILD_INTERFACE:${leveldb_SOURCE_DIR}/include>
            )
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(AppleClang|Clang|GNU)$")
    target_add_flag(leveldb -Wno-unused-parameter)
    target_add_flag(leveldb -Wno-implicit-fallthrough)
    target_add_flag(leveldb -Wno-format-nonliteral)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
endif()


add_compile_definitions(WITH_LEVELDB)