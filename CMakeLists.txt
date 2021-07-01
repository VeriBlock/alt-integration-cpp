cmake_minimum_required(VERSION 3.12)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
endif()

if(WIN32)
    find_program(buildcache_program buildcache)
    if(buildcache_program)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${buildcache_program}")
    endif()
else()
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)
endif()

set(LIB_NAME veriblock-pop-cpp)
project(${LIB_NAME} LANGUAGES C CXX)

include(cmake/GitVersion.cmake)
include(GNUInstallDirs)
include(CheckCCompilerFlag)
include(ProcessorCount)
include(CheckCXXCompilerFlag)
include(cmake/asan.cmake)
include(cmake/tsan.cmake)
include(cmake/ubsan.cmake)
include(cmake/functions.cmake)

ProcessorCount(N_PROCESSORS)

has_extrinsic("int main(){__builtin_expect(0, 1);return 0;}"  VBK_HAS_BUILTIN_EXPECT)
has_extrinsic("int main(){__builtin_clz(5);      return 0;}"  VBK_HAS_BUILTIN_CLZ)
has_extrinsic("int main(){__builtin_popcount(5); return 0;}"  VBK_HAS_BUILTIN_POPCOUNT)
has_extrinsic("
    void f(float * __restrict__ a){}
    int main(){return 0;}
" VBK_HAS_RESTRICT)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_BUILD_TYPE Debug)
endif()

option(CLANG_TIDY   "Enable clang-tidy checks during compilation" OFF)
option(ASAN         "Enable Address Sanitizer" OFF)
option(TSAN         "Enable Thread Sanitizer" OFF)
option(UBSAN        "Enable Undefined Behavior Sanitizer" OFF)
option(COVERAGE     "Enable coverage" OFF)
option(WERROR       "Treat warnings as errors" ON)
option(FUZZING      "Build fuzz" OFF)
option(TESTING      "Build tests" ON)
option(TESTING_GO   "Run GO bindings tests with ctest" OFF)
option(BENCHMARKING "Build benchmarks" OFF)
option(SHARED       "Build shared lib" OFF)
option(WITH_SECP256K1 "Include secp256k1" ON)
option(VERIBLOCK_POP_LOGGER_ENABLED "Use logger" ON)
option(WITH_PYPOPTOOLS "Build pypoptools" OFF)
option(VBK_NO_THREADS  "Build PopValidator without std::threads" OFF)
option(WITH_C_WRAPPER  "Build the c wrapper" OFF)
option(WITH_ROCKSDB   "Build with the rocksdb storage" OFF)
option(WITH_LEVELDB  "Build with the leveldb storage" OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

## setup compilation flags
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(AppleClang|Clang|GNU)$")
    if(WERROR)
        add_compile_options(-Werror)
    endif()
    # enable those flags
    add_flag(-Wall)
    add_flag(-Wextra)
    add_cxx_flag(-Woverloaded-virtual) # warn if you overload (not override) a virtual function
    add_cxx_flag(-Wnon-virtual-dtor)   # warn the user if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors
    add_flag(-Wdouble-promotion)       # (GCC >= 4.6, Clang >= 3.8) warn if float is implicit promoted to double
    add_flag(-Wduplicated-cond)        # (only in GCC >= 6.0) warn if if / else chain has duplicated conditions
    add_flag(-Wduplicated-branches)    # (only in GCC >= 7.0) warn if if / else branches have duplicated code
    add_flag(-Wformat=2)               # warn on security issues around functions that format output (ie printf)
    add_flag(-Wnull-dereference)       # (only in GCC >= 6.0) warn if a null dereference is detected
    add_flag(-Wsign-compare)
    add_flag(-Wunused-lambda-capture)  # error if lambda capture is unused
    add_flag(-Wunused-local-typedef)

    # disable errors
    add_cxx_flag(-Wno-error=literal-suffix)

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        # Release flags
        add_flag(-Ofast)
    else()
        # Debug flags
        add_flag(-Og)
        add_flag(-g)
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # TODO: enable and fix warnings
#    if(WERROR)
#        add_compile_options(/WX)                      # treats all compiler warnings as errors.
#    endif()

    # using Visual Studio C++
    add_flag(/W4)                      # displays level 1, level 2, and level 3 warnings, and all level 4 (informational) warnings that are not turned off by default.
    add_flag(/permissive-)             # enforces standards conformance
    add_flag(/w14242)                  # 'identfier': conversion from 'type1' to 'type1', possible loss of data
    add_flag(/w14263)                  # 'function': member function does not override any base class virtual member function
    add_flag(/w14265)                  # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
    add_flag(/w14296)                  # 'operator': expression is always 'boolean_value'
    add_flag(/w14826)                  # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
    add_flag(/w14640)                  # Enable warning on thread un-safe static member initialization

    # disable these warnings:
    # appears in the gtest library
    add_flag(/wd4389)                  # disable warning: 'operator' : signed/unsigned mismatch
    add_flag(/wd4455)                  # disable warning: 'operator ""s': literal suffix identifiers that do not start with an underscore are reserved
    add_flag(/wd4127)                  # disable warning: conditional expression is constant
    add_flag(/wd4505)                  # disable warning: warning C4505: 'dummy': unreferenced local function has been removed

    # https://stackoverflow.com/questions/5004858/why-is-stdmin-failing-when-windows-h-is-included
    add_flag(/DNOMINMAX)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    if(SHARED)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd") 
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    endif()
endif()

include(FetchContent)
if(NOT VERIBLOCK_POP_LOGGER_ENABLED)
    vbk_define(VERIBLOCK_POP_LOGGER_DISABLED=1)
endif()
vbk_define(FMT_HEADER_ONLY=1)

if(CLANG_TIDY)
    include(cmake/clang-tidy.cmake)
endif()
if(COVERAGE)
    include(cmake/coverage.cmake)
endif()

if(TESTING)
    include(cmake/gtest.cmake)
    enable_testing()
endif()

if(WITH_ROCKSDB)
    if(WITH_LEVELDB)
        message(FATAL_ERROR "Should specify only one storage implementation, shoose on from the list: -DWITH_LEVELDB=ON, -DWITH_ROCKSDB=ON")
    endif()
    include(cmake/rocksdb.cmake)
endif()
if(WITH_LEVELDB)
    if(WITH_ROCKSDB)
        message(FATAL_ERROR "Should specify only one storage implementation, shoose on from the list: -DWITH_LEVELDB=ON, -DWITH_ROCKSDB=ON")
    endif()
    include(cmake/leveldb.cmake)
endif()

include_directories(include)
add_subdirectory(src)
add_subdirectory(bindings EXCLUDE_FROM_ALL)
add_subdirectory(tools EXCLUDE_FROM_ALL)

if(TESTING)
    add_subdirectory(test)
endif()
if(BENCHMARKING)
    add_subdirectory(benchmark)
endif()
if(WITH_PYPOPTOOLS)
    add_subdirectory(pypoptools)
endif()

message(STATUS "CLANG_TIDY=${CLANG_TIDY}")
message(STATUS "ASAN=${ASAN}")
message(STATUS "TSAN=${TSAN}")
message(STATUS "UBSAN=${UBSAN}")
message(STATUS "COVERAGE=${COVERAGE}")
message(STATUS "FUZZING=${FUZZING}")
message(STATUS "WERROR=${WERROR}")
message(STATUS "TESTING=${TESTING}")
message(STATUS "TESTING_GO=${TESTING_GO}")
message(STATUS "WITH_PYPOPTOOLS=${WITH_PYPOPTOOLS}")
message(STATUS "WITH_C_WRAPPER=${WITH_C_WRAPPER}")
message(STATUS "WITH_ROCKSDB=${WITH_ROCKSDB}")
message(STATUS "WITH_LEVELDB=${WITH_LEVELDB}")
message(STATUS "BENCHMARKING=${BENCHMARKING}")
message(STATUS "CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")
if(WIN32)
    message(STATUS "WIN32_BUILDCACHE_PROGRAM=${buildcache_program}")
else()
    message(STATUS "CCACHE_PROGRAM=${CCACHE_FOUND}")
endif()

if(FUZZING)
    add_subdirectory(fuzz)
endif()