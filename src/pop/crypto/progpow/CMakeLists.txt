add_subdirectory(libethash)

add_library(progpow OBJECT
        cache_entry.cpp
        kiss99.cpp
        progpow.cpp
        math.cpp
        )
enable_clang_tidy(progpow)
