function(enable_tsan_on_target target)
    if(TSAN)
        target_compile_options(${target} PRIVATE
                -g -fsanitize=thread
                )
        target_link_options(${target} PRIVATE
                -fsanitize=thread
                )
    endif()
endfunction()
