add_subdirectory(pop)
add_subdirectory(commands)

add_library(blockchain OBJECT
        alt_block_tree.cpp
        btc_blockchain_util.cpp
        vbk_blockchain_util.cpp
        )