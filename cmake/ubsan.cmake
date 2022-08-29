function(enable_ubsan_on_target target)
    if(UBSAN)
        target_compile_options(${target} PRIVATE
                -fsanitize=undefined
                -fno-omit-frame-pointer
                -g
                )
        target_link_options(${target} PRIVATE
                -fsanitize=undefined
                )
        set(ENV{UBSAN_OPTIONS} print_stacktrace=1)
    endif()
endfunction()
