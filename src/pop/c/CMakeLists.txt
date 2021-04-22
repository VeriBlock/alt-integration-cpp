add_subdirectory(adaptors)

add_library(veriblock-pop-c OBJECT
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
        )
target_link_libraries(veriblock-pop-c
        rocksdb
        )