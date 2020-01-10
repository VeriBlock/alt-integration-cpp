cmake_minimum_required(VERSION 3.5)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)


project(alt-integration C CXX)

include(CheckCXXCompilerFlag)
include(cmake/functions.cmake)
include(cmake/dependencies.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TESTING "Build tests" ON)

## setup compilation flags
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(AppleClang|Clang|GNU)$")
    # enable those flags
    add_flag(-Wall)
    add_flag(-Werror)
    add_flag(-Wextra)
    add_flag(-Woverloaded-virtual)     # warn if you overload (not override) a virtual function
    add_flag(-Wformat=2)               # warn on security issues around functions that format output (ie printf)
    add_flag(-Wmisleading-indentation) # (only in GCC >= 6.0) warn if indentation implies blocks where blocks do not exist
    add_flag(-Wduplicated-cond)        # (only in GCC >= 6.0) warn if if / else chain has duplicated conditions
    add_flag(-Wduplicated-branches)    # (only in GCC >= 7.0) warn if if / else branches have duplicated code
    add_flag(-Wnull-dereference)       # (only in GCC >= 6.0) warn if a null dereference is detected
    add_flag(-Wdouble-promotion)       # (GCC >= 4.6, Clang >= 3.8) warn if float is implicit promoted to double
    add_flag(-Wsign-compare)
    add_flag(-Wtype-limits)            # size_t - size_t >= 0 -> always true

    # promote to errors
    add_flag(-Werror-unused-lambda-capture)  # error if lambda capture is unused
    add_flag(-Werror-return-type)      # warning: control reaches end of non-void function [-Wreturn-type]
    add_flag(-Werror-non-virtual-dtor) # warn the user if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors
    add_flag(-Werror-sign-compare)     # warn the user if they compare a signed and unsigned numbers
    add_flag(-Werror-reorder)          # field '$1' will be initialized after field '$2'
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++
    add_flag(/W4)                      # displays level 1, level 2, and level 3 warnings, and all level 4 (informational) warnings that are not turned off by default.
    add_flag(/WX)                      # treats all compiler warnings as errors.
    add_flag(/permissive-)             # enforces standards conformance
    add_flag(/w14242)                  # 'identfier': conversion from 'type1' to 'type1', possible loss of data
    add_flag(/w14263)                  # 'function': member function does not override any base class virtual member function
    add_flag(/w14265)                  # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not be destructed correctly
    add_flag(/w14296)                  # 'operator': expression is always 'boolean_value'
    add_flag(/w14826)                  # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
    add_flag(/w14640)                  # Enable warning on thread un-safe static member initialization

    # disable these warnings:
    add_flag(/wd4389)                  # disable warning: 'operator' : signed/unsigned mismatch
endif()

include(GNUInstallDirs)

include_directories(include)

add_subdirectory(src)

if(TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
