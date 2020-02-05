include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(third_party)
add_subdirectory(entities)

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
        $<TARGET_OBJECTS:entities>
        $<TARGET_OBJECTS:secp256k1>
        $<TARGET_OBJECTS:sha256>
        $<TARGET_OBJECTS:vblake>
        )

if(WITH_ROCKSDB)
    target_link_libraries(vbkutils RocksDB::rocksdb)
endif()
