add_subdirectory(pop)

add_library(blockchain OBJECT
        alt_block_tree.cpp
        btc_blockchain_util.cpp
        vbk_blockchain_util.cpp
        alt_block_addon.cpp
        vbk_block_addon.cpp
        btc_block_addon.cpp
        commands.cpp
        mempool_block_tree.cpp
        commands/alt_command_group_store.cpp
        commands/check_pubdata.cpp
        commands/vbk_command_group_store.cpp
        )
