function(enable_asan_on_target target)
    if(ASAN)
        target_compile_options(${target} PRIVATE
                -fsanitize=address
                -fsanitize-address-use-after-scope
                -g
                -O1
                -DNDEBUG
                -Wno-error=null-dereference
                )
        target_link_options(${target} PRIVATE
                -fsanitize=address
                )
        set(ENV{ASAN_OPTIONS} detect_container_overflow=0:verbosity=1:debug=1:detect_leaks=1:check_initialization_order=1:alloc_dealloc_mismatch=true:use_odr_indicator=true)
    endif()
endfunction()
