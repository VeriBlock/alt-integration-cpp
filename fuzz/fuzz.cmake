if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
    message(FATAL_ERROR "
Compilers other than clang are not supported.
Clean build dir and try this:
  CC=clang CXX=clang++ cmake .. -DFUZZING=ON
")
endif()

if(NOT DEFINED ENV{VBK_FUZZ_CORPUS_DIR})
    message(FATAL_ERROR "
Please clone https://github.com/VeriBlock/fuzz-corpus and define
VBK_FUZZ_CORPUS_DIR environment variable with path to fuzz-corpus!
    ")
endif()

function(fuzz_add_cov_flags target)
    if(FUZZ_COV)
        target_compile_options(${target} PUBLIC -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(${target} PUBLIC -fprofile-instr-generate -fcoverage-mapping)
    endif()
endfunction()


fuzz_add_cov_flags(${POP_LIB_NAME})


add_custom_target(fuzz
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
        DEPENDS fuzz_targets
        )
add_custom_command(
        OUTPUT fuzz_targets
        DEPENDS ${POP_LIB_NAME}
        COMMAND ;
)
set_source_files_properties(fuzz_targets PROPERTIES SYMBOLIC "true")


function(add_fuzz FUZZ_TARGET)
    set(options)
    set(oneValueArgs CORPUS_DIR TIMEOUT WEIGHT)
    set(multiValueArgs SOURCES ARGS)
    cmake_parse_arguments(FUZZ
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN})

    if(FUZZ_TIMEOUT)
        MATH(EXPR THIS_FUZZ_TIMEOUT "${FUZZ_TIMEOUT} * ${FUZZ_WEIGHT} / ${TOTAL_WEIGHT}")
        set(FUZZ_TIMEOUT -max_total_time=${THIS_FUZZ_TIMEOUT})
        message(STATUS "${FUZZ_TARGET} will run for ${THIS_FUZZ_TIMEOUT} sec")
    endif()
    if(NOT FUZZ_MAX_LEN)
        set(FUZZ_MAX_LEN 4096)
    endif()
    if(NOT FUZZ_PROCESSORS AND (PROCESSOR_COUNT EQUAL 0 OR NOT PROCESSOR_COUNT))
        set(FUZZ_PROCESSORS 1)
    endif()
    if(FUZZ_CORPUS_DIR)
        file(MAKE_DIRECTORY ${FUZZ_CORPUS_DIR})
    endif()
    if(FUZZ_ONCE)
        set(FUZZ_ONCE -runs=1)
    endif()

    add_executable(${FUZZ_TARGET} ${FUZZ_SOURCES})
    fuzz_add_cov_flags(${FUZZ_TARGET})
    if(ASAN)
        set(fuzzers fuzzer,address)
    elseif(UBSAN)
        set(fuzzers fuzzer,undefined)
    elseif(TSAN)
        set(fuzzers fuzzer,thread)
    else()
        set(fuzzers fuzzer)
    endif()

    set_target_properties(${FUZZ_TARGET} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED TRUE
            )
    target_link_libraries(${FUZZ_TARGET} PUBLIC ${POP_LIB_NAME})
    target_compile_options(${FUZZ_TARGET} PUBLIC
            -fsanitize=${fuzzers}
            -g
            )
    target_link_options(${FUZZ_TARGET} PRIVATE
            -fsanitize=${fuzzers}
            )
    set(ccov_dir ${CMAKE_BINARY_DIR}/ccov)
    file(MAKE_DIRECTORY ${ccov_dir})
    file(MAKE_DIRECTORY $ENV{VBK_FUZZ_CORPUS_DIR}/${FUZZ_TARGET})
    add_custom_target(run_${FUZZ_TARGET}
            WORKING_DIRECTORY ${ccov_dir}
            DEPENDS ${FUZZ_TARGET}
            COMMAND ${CMAKE_EXECUTE_PROCESS_COMMAND_ECHO}
            COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${ccov_dir}/${FUZZ_TARGET}.profraw $<TARGET_FILE:${FUZZ_TARGET}>
                ${FUZZ_TIMEOUT}
                -max_len=${FUZZ_MAX_LEN}
                -print_final_stats=1
                -fork=${PROCESSOR_COUNT}
                ${FUZZ_ONCE}
                ${ARGS}
                $ENV{VBK_FUZZ_CORPUS_DIR}/${FUZZ_TARGET}
                USES_TERMINAL
                VERBATIM
            )
    add_custom_command(
            OUTPUT fuzz_targets
            COMMENT "Running ${FUZZ_TARGET}"
            DEPENDS run_${FUZZ_TARGET}
            APPEND
    )
    add_test(
            NAME ${FUZZ_TARGET}
            COMMAND $<TARGET_FILE:${FUZZ_TARGET}> -runs=1 ${FUZZ_CORPUS_DIR}
    )
endfunction()
