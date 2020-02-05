include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(third_party)

add_library(vblake OBJECT vblake.cpp)

add_library(vbkutils
        checks.cpp
        serde.cpp
        strutil.cpp
        read_stream.cpp
        write_stream.cpp
        base58.cpp
        base59.cpp
        hashutil.cpp
        stateless_validation.cpp
        arith_uint256.cpp
        signutil.cpp
        entities/address.cpp
        entities/atv.cpp
        entities/btcblock.cpp
        entities/btctx.cpp
        entities/coin.cpp
        entities/merkle_path.cpp
        entities/output.cpp
        entities/publication_data.cpp
        entities/vbk_merkle_path.cpp
        entities/vbkblock.cpp
        entities/vbkpoptx.cpp
        entities/vbktx.cpp
        entities/vtb.cpp
        $<TARGET_OBJECTS:secp256k1>
        $<TARGET_OBJECTS:sha256>
        $<TARGET_OBJECTS:vblake>
        )

if(WITH_ROCKSDB)
    target_link_libraries(vbkutils rocksdb)
    target_include_directories(vbkutils PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../deps/rocksdb/include>)
endif()
