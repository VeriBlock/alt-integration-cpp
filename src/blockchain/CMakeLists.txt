add_subdirectory(pop)

add_library(blockchain OBJECT
        alt_block_tree.cpp
        btc_blockchain_util.cpp
        vbk_blockchain_util.cpp
        )

if(WITH_ROCKSDB)
    target_link_libraries(blockchain rocksdb)
endif()