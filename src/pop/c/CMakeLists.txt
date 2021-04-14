add_subdirectory(adaptors)

add_library(veriblock-pop-c OBJECT
        config.cpp
        pop_context.cpp
        utils.cpp
        bytestream.cpp
        mock_miner.cpp
        validation_state.cpp
        storage.cpp
        # entities
        entities/btcblock.cpp
        entities/vbkblock.cpp
        entities/altblock.cpp
        array.cpp
        )
target_link_libraries(veriblock-pop-c
        rocksdb
        )