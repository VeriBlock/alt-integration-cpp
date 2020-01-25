include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(third_party)

add_library(vblake OBJECT vblake.cpp)

add_library(vbkutils
        uint.cpp
        checks.cpp
        serde.cpp
        strutil.cpp
        read_stream.cpp
        write_stream.cpp
        base58.cpp
        base59.cpp
        hashutil.cpp
        signutil.cpp
        $<TARGET_OBJECTS:secp256k1>
        $<TARGET_OBJECTS:sha256>
        $<TARGET_OBJECTS:vblake>
        stateless_validation.cpp
        )
