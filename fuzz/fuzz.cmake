if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    message(FATAL_ERROR "
Compilers other than clang are not supported.
Clean build dir and try this:
  CC=clang CXX=clang++ cmake .. -DFUZZING=ON
")
endif()

target_compile_options(${LIB_NAME} PUBLIC -fprofile-instr-generate -fcoverage-mapping)
target_link_options(${LIB_NAME} PUBLIC -fprofile-instr-generate -fcoverage-mapping)

add_custom_target(fuzz
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
        DEPENDS fuzz_targets
        )
add_custom_command(
        OUTPUT fuzz_targets
        DEPENDS ${LIB_NAME}
        COMMAND ;
)
set_source_files_properties(fuzz_targets PROPERTIES SYMBOLIC "true")


function(add_fuzz FUZZ_TARGET)
    set(options)
    set(oneValueArgs CORPUS_DIR TIMEOUT RUNS MAX_LEN)
    set(multiValueArgs SOURCES ARGS)
    cmake_parse_arguments(FUZZ
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN})

    if(FUZZ_TIMEOUT)
        set(FUZZ_TIMEOUT -max_total_time=${FUZZ_TIMEOUT})
    endif()
    if(NOT FUZZ_MAX_LEN)
        set(FUZZ_MAX_LEN 4096)
    endif()
    if(N_PROCESSORS EQUAL 0 OR NOT N_PROCESSORS)
        set(FUZZ_PROCESSORS 1)
    endif()
    if(FUZZ_CORPUS_DIR)
        file(MAKE_DIRECTORY ${FUZZ_CORPUS_DIR})
    endif()

    add_executable(${FUZZ_TARGET} ${FUZZ_SOURCES})
    target_link_libraries(${FUZZ_TARGET} ${LIB_NAME})
    target_compile_options(${FUZZ_TARGET} PRIVATE
            -fsanitize=fuzzer,address
            -g
            )
    target_link_options(${FUZZ_TARGET} PRIVATE
            -fsanitize=fuzzer,address
            )
    add_custom_command(
            OUTPUT fuzz_targets APPEND
            COMMENT "Running ${FUZZ_TARGET}"
            COMMAND ${FUZZ_TARGET} ARGS
            ${FUZZ_TIMEOUT}
            -max_len=${FUZZ_MAX_LEN}
            -print_final_stats=1
            -fork=${N_PROCESSORS}
            ${ARGS}
            ${FUZZ_CORPUS_DIR}
            USES_TERMINAL
            VERBATIM
    )

    add_test(
            NAME ${FUZZ_TARGET}
            COMMAND ${FUZZ_TARGET} -runs=1 ${FUZZ_CORPUS_DIR}
    )
endfunction()