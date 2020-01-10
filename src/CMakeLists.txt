add_library(vblake vblake.cpp)
add_library(vbkutils
        serde.cpp
        strutil.cpp
        read_stream.cpp
        write_stream.cpp
        crypto/base58.cpp
        )
