function(disable_clang_tidy target)
    set_target_properties(${target} PROPERTIES
            C_CLANG_TIDY ""
            CXX_CLANG_TIDY ""
            )
endfunction()

function(addtest test_name)
    add_executable(${test_name} ${ARGN})
    addtest_part(${test_name} ${ARGN})
    target_link_libraries(${test_name}
            GTest::Main
            GTest::GTest
            )
    add_test(
            NAME ${test_name}
            COMMAND $<TARGET_FILE:${test_name}>
    )
    set_target_properties(${test_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin
            ARCHIVE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/test_lib
            LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/test_lib
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED TRUE
            )
    disable_clang_tidy(${test_name})
endfunction()

function(addtest_part test_name)
    if (POLICY CMP0076)
        cmake_policy(SET CMP0076 NEW)
    endif ()
    target_sources(${test_name} PUBLIC
            ${ARGN}
            )
    target_link_libraries(${test_name}
            GTest::GTest
            )
endfunction()

# conditionally applies flag. If flag is supported by current compiler, it will be added to compile options.
function(add_flag flag)
    check_cxx_compiler_flag(${flag} CXXFLAG_${flag})
    if (CXXFLAG_${flag} EQUAL 1)
        add_compile_options(${flag})
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