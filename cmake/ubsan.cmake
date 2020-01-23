if (DEFINED FLAGS_SANITIZE_UNDEFINED_CMAKE_)
    return()
else ()
    set(FLAGS_SANITIZE_UNDEFINED_CMAKE_ 1)
endif ()

set(FLAGS
        -fsanitize=undefined
        -fno-omit-frame-pointer
        -g
        )
foreach(FLAG IN LISTS FLAGS)
    add_cache_flag(CMAKE_CXX_FLAGS ${FLAG})
    add_cache_flag(CMAKE_C_FLAGS ${FLAG})
endforeach()


set(ENV{UBSAN_OPTIONS} print_stacktrace=1)
