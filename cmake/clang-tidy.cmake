function(enable_clang_tidy target)
    if (CLANG_TIDY)
        find_program(CLANG_TIDY_BIN clang-tidy REQUIRED)

        set(__TIDY_CMD "${CLANG_TIDY_BIN}")

        set_target_properties(${target} PROPERTIES
            C_CLANG_TIDY   ${__TIDY_CMD}
            CXX_CLANG_TIDY ${__TIDY_CMD}
        )
        message(STATUS "Enable clang-tidy on ${target}")
    endif()
endfunction()
