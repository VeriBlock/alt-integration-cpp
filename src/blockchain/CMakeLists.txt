add_subdirectory(pop)

add_library(blockchain OBJECT
        alt_block_tree.cpp
        btc_blockchain_util.cpp
        vbk_blockchain_util.cpp
        alt_chain_params_serializable.cpp
        btc_chain_params_serializable.cpp
        vbk_chain_params_serializable.cpp
	blockchain_storage_util.cpp
        )
