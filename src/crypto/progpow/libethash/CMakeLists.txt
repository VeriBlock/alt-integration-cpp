add_library(ethash OBJECT
        internal.c
        sha3.c
        data_sizes.c
        )
disable_clang_tidy(ethash)

if(NOT MSVC)
    target_compile_options(ethash PRIVATE -std=gnu99)
endif()