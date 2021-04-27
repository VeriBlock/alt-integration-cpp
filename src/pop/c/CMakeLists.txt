set(C_SOURCES
        config.cpp
        pop_context.cpp
        utils.cpp
        bytestream.cpp
        mock_miner.cpp
        validation_state.cpp
        storage.cpp
        array.cpp
        mock_miner2.cpp
        validation_state2.cpp
        storage2.cpp
        config2.cpp
        pop_context2.cpp
        # entities
        entities/publication_data.cpp
        entities/btcblock.cpp
        entities/vbkblock.cpp
        entities/altblock.cpp
        entities/atv.cpp
        entities/vtb.cpp
        entities/popdata.cpp
        entities/network_byte_pair.cpp
        entities/address.cpp
        entities/btctx.cpp
        entities/coin.cpp
        # adaptors
        adaptors/block_provider_impl.cpp
        )

if(NOT WITH_ROCKSDB AND NOT WITH_LEVELDB)
        message(FATAL_ERROR "For the c wrapper you should specify the storage which you will build, -DWITH_ROCKSDB=ON or -DWITH_LEVELDB=ON ")
endif()
        
if(WITH_ROCKSDB)
        list(APPEND C_SOURCES adaptors/rocksdb_impl.cpp)
endif()

if(WITH_LEVELDB)
        list(APPEND C_SOURCES adaptors/leveldb_impl.cpp)
endif()

add_library(veriblock-pop-c OBJECT ${C_SOURCES})

if(WITH_ROCKSDB)
        target_link_libraries(veriblock-pop-c rocksdb)
endif()

if(WITH_LEVELDB)
        target_link_libraries(veriblock-pop-c leveldb)
endif()