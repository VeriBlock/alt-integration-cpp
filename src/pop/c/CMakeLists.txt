set(C_SOURCES
        array.cpp
        mock_miner.cpp
        validation_state.cpp
        storage.cpp
        config.cpp
        pop_context.cpp
        utils.cpp
        mempool.cpp
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
        entities/merkle_path.cpp
        entities/block_index.cpp
        entities/pop_payouts.cpp
        entities/context_info_container.cpp
        )

if(NOT WITH_ROCKSDB AND NOT WITH_LEVELDB)
        message(FATAL_ERROR "For the c wrapper you should specify the storage which you will build, -DWITH_ROCKSDB=ON or -DWITH_LEVELDB=ON ")
endif()

add_library(veriblock-pop-c OBJECT ${C_SOURCES})
enable_clang_tidy(veriblock-pop-c)
