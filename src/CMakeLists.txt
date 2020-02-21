include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(blockchain)
add_subdirectory(third_party)
add_subdirectory(entities)
add_subdirectory(storage)

add_library(vblake OBJECT vblake.cpp)

add_library(vbkutils
        checks.cpp
        consts.cpp
        serde.cpp
        strutil.cpp
        keystone_util.cpp
        read_stream.cpp
        write_stream.cpp
        base58.cpp
        base59.cpp
        hashutil.cpp
        stateless_validation.cpp
        arith_uint256.cpp
        signutil.cpp
        mock_miner.cpp
        $<TARGET_OBJECTS:blockchain>
        $<TARGET_OBJECTS:pop_utils>
        $<TARGET_OBJECTS:entities>
        $<TARGET_OBJECTS:storage>
        $<TARGET_OBJECTS:secp256k1>
        $<TARGET_OBJECTS:sha256>
        $<TARGET_OBJECTS:vblake>
        )