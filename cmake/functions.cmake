function(vbk_define def)
    if(UNIX)
        add_definitions(-D${def})
    else()
        add_definitions(/D${def})
    endif()
endfunction()


function(enable_werror target)
    if(WERROR)
        if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(AppleClang|Clang|GNU)$")
            target_compile_options(${target} PRIVATE -Werror)
        elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            target_compile_options(${target} PRIVATE /WX)
        endif()
    endif()
endfunction()


function(disable_clang_tidy target)
    set_target_properties(${target} PROPERTIES
            C_CLANG_TIDY ""
            CXX_CLANG_TIDY ""
            )
endfunction()

function(set_test_cost target cost)
    set_tests_properties(${target} PROPERTIES
            COST ${cost}
            )
endfunction()

add_custom_target(all_tests)

function(addtest test_name)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    add_executable(
            ${test_name}
            ${ARGN}
            $<TARGET_OBJECTS:test_main>
    )
    addtest_part(${test_name} ${ARGN})
    target_link_libraries(${test_name}
            gtest
            Threads::Threads
            )
    add_test(
            NAME ${test_name}
            COMMAND $<TARGET_FILE:${test_name}>
    )
    set_target_properties(${test_name} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED TRUE
            )
    disable_clang_tidy(${test_name})
    if(UNIX)
        # works only on UNIX systems
        target_compile_options(${test_name} PUBLIC
                # we don't care about potential null dereferences in tests
                -Wno-null-dereference
                )
    endif()
    add_dependencies(all_tests ${test_name})
    enable_asan_on_target(${test_name})
    enable_tsan_on_target(${test_name})
    enable_ubsan_on_target(${test_name})
    enable_stacktrace_on_target(${test_name})
    enable_werror(${test_name})
endfunction()

function(addtest_part test_name)
    if (POLICY CMP0076)
        cmake_policy(SET CMP0076 NEW)
    endif ()
    target_sources(${test_name} PUBLIC
            ${ARGN}
            )
endfunction()

function(target_add_flag target flag)
    check_cxx_compiler_flag(${flag} CXXFLAG_${flag})
    if (CXXFLAG_${flag} EQUAL 1)
        target_compile_options(${target} PRIVATE ${flag})
    endif ()
endfunction()

# conditionally applies flag. If flag is supported by current compiler, it will be added to compile options.
function(add_flag flag)
    check_cxx_compiler_flag(${flag} CXXFLAG_${flag})
    if (CXXFLAG_${flag} EQUAL 1)
        add_compile_options(${flag})
    endif ()
endfunction()

function(target_add_flag_visibility target visibility flag)
    check_cxx_compiler_flag(${flag} CXXFLAG_${flag})
    if (CXXFLAG_${flag} EQUAL 1)
        target_compile_options(${target} ${visibility} ${flag})
    endif ()
endfunction()

function(add_c_flag flag)
    check_c_compiler_flag(${flag} CFLAG_${flag})
    if (CFLAG_${flag} EQUAL 1)
        add_cache_flag(CMAKE_C_FLAGS ${flag})
    endif ()
endfunction()

function(add_cxx_flag flag)
    check_cxx_compiler_flag(${flag} CXXFLAG_${flag})
    if (CXXFLAG_${flag} EQUAL 1)
        add_cache_flag(CMAKE_CXX_FLAGS ${flag})
    endif ()
endfunction()

function(add_cache_flag var_name flag)
    set(spaced_string " ${${var_name}} ")
    string(FIND "${spaced_string}" " ${flag} " flag_index)
    if(NOT flag_index EQUAL -1)
        return()
    endif()
    string(COMPARE EQUAL "" "${${var_name}}" is_empty)
    if(is_empty)
        # beautify: avoid extra space at the end if var_name is empty
        set("${var_name}" "${flag}" CACHE STRING "" FORCE)
    else()
        set("${var_name}" "${flag} ${${var_name}}" CACHE STRING "" FORCE)
    endif()
endfunction()

# reads file contents and creates a single cpp in ${CMAKE_CURRENT_BINARY_DIR}/${varname}.cpp
## varname[in] - variable name, as well as file name
## infile[in] - input file path
## out[out] - returns full path to generated file
##
####################################################
## add this to hpp, and include this hpp to access
## the variable.
####################################################
## #pragma once
## namespace generated {
##   extern const char ${varname}[];
## }
##
function(gencpp varname infile out)
    get_filename_component(_absin ${infile} REALPATH)
    set(path ${varname}.cpp)
    set(${out} ${path} PARENT_SCOPE)

    add_custom_command(
            OUTPUT ${path}
            COMMAND filetoheader
            ARGS
            ${varname}
            ${_absin}
            ${path}
            DEPENDS filetoheader
            COMMENT "Generating ${varname}.cpp..."
    )
endfunction()


if(UNIX)
    message(STATUS "Adding target 'uninstall'")

    add_custom_target(uninstall
            COMMAND xargs rm < ${CMAKE_BINARY_DIR}/install_manifest.txt
            COMMENT "Uninstalling altintegration..."
            )

endif()

function(has_extrinsic code OUT)
    check_cxx_source_compiles("${code}" ${OUT})
    if(${OUT})
        vbk_define(${OUT})
    endif()
endfunction()


