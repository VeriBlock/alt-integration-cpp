add_subdirectory(libethash)

add_library(progpow OBJECT
        kiss99.cpp
        progpow.cpp
        math.cpp
        )