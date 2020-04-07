add_subdirectory(pop)

add_library(blockchain OBJECT
        # TODO: fix test
        # alt_block_tree.cpp
        btc_blockchain_util.cpp
        vbk_blockchain_util.cpp
        )