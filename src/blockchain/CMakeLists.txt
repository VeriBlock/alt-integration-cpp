add_subdirectory(pop)

add_library(blockchain OBJECT
        btc_blockchain_util.cpp
        vbk_blockchain_util.cpp
        )