include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(secp256k1)

add_library(vblake vblake.cpp)
add_library(vbkutils
        serde.cpp
        strutil.cpp
        read_stream.cpp
        write_stream.cpp
        base58.cpp
        base59.cpp
        sha256.cpp
        hashutil.cpp
        stateless_validation.cpp
        )
