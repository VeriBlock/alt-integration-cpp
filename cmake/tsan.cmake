if (DEFINED FLAGS_SANITIZE_THREAD_CMAKE_)
    return()
else ()
    set(FLAGS_SANITIZE_THREAD_CMAKE_ 1)
endif ()

add_cache_flag(CMAKE_CXX_FLAGS "-fsanitize=thread")
add_cache_flag(CMAKE_CXX_FLAGS "-g")

add_cache_flag(CMAKE_C_FLAGS "-fsanitize=thread")
add_cache_flag(CMAKE_C_FLAGS "-g")

add_cache_flag(CMAKE_EXE_LINKER_FLAGS "-fsanitize=thread")
add_cache_flag(CMAKE_SHARED_LINKER_FLAGS "-fsanitize=thread")
