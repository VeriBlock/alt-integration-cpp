add_subdirectory(adaptors)

add_library(veriblock-pop-c OBJECT
        config.cpp
        pop_context.cpp
        utils.cpp
        bytestream.cpp
        mock_miner.cpp
        validation_state.cpp
        )
target_link_libraries(veriblock-pop-c
        rocksdb
        )