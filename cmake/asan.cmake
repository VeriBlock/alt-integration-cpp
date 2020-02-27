if (DEFINED FLAGS_SANITIZE_ADDRESS_CMAKE_)
    return()
else ()
    set(FLAGS_SANITIZE_ADDRESS_CMAKE_ 1)
endif ()

set(FLAGS
        -fsanitize=address
        -fsanitize-address-use-after-scope
        -g
        -O1
        -DNDEBUG
        -Wno-error=null-dereference
        )
foreach(FLAG IN LISTS FLAGS)
    add_cache_flag(CMAKE_CXX_FLAGS ${FLAG})
    add_cache_flag(CMAKE_C_FLAGS ${FLAG})
endforeach()

set(ENV{ASAN_OPTIONS} verbosity=1:debug=1:detect_leaks=1:check_initialization_order=1:alloc_dealloc_mismatch=true:use_odr_indicator=true)
