add_subdirectory(pop)

gencpp(
        btc_blockheaders_mainnet_0_10000
        btc_blockheaders_mainnet_0_10000.txt
        btc_gen_1
)

gencpp(
        btc_blockheaders_testnet_0_10000
        btc_blockheaders_testnet_0_10000.txt
        btc_gen_2
)

gencpp(
        vbk_blockheaders_mainnet_200001_230000
        vbk_blockheaders_mainnet_200001_230000.txt
        vbk_gen_1
)

gencpp(
        vbk_blockheaders_testnet_0_10000
        vbk_blockheaders_testnet_0_10000.txt
        vbk_gen_2
)

gencpp(
        vbk_testnet30000
        vbk_testnet30000.txt
        vbk_blocks_gen_1
)

addtest(btc_blockchain_test
        btc_blockchain_test.cpp
        # generated cpps
        ${btc_gen_1}
        ${btc_gen_2}
        )

addtest(vbk_blockchain_test
        vbk_blockchain_test.cpp
        # generated cpps
        ${vbk_gen_1}
        ${vbk_gen_2}
        )
set_tests_properties(vbk_blockchain_test PROPERTIES
        COST 300000  # many sec
        )

addtest(vbk_getblockproof_test
        vbk_getblockproof_test.cpp
        ${vbk_blocks_gen_1}
        )
set_tests_properties(vbk_getblockproof_test PROPERTIES
        COST 30000 # 30 sec
        )

addtest(chain_test chain_test.cpp)
addtest(chainslice_test chainslice_test.cpp)
addtest(blockchain_test blockchain_test.cpp)
set_test_cost(blockchain_test 105000) # 105 sec
addtest(chainparams_test chainparams_test.cpp)
addtest(alt_blockchain_test alt_blockchain_test.cpp)
addtest(alt_invalidation_test alt_invalidation_test.cpp)
addtest(btc_invalidation_test btc_invalidation_test.cpp)
addtest(vbk_blockchain_util_test vbk_blockchain_util_test.cpp)
addtest(temp_block_tree_test temp_block_tree_test.cpp)
addtest(alt_tree_bootstrap_test alt_tree_bootstrap_test.cpp)
addtest(alttree_finalization_test alttree_finalization_test.cpp)
# TODO(warchant): fix finalization
# addtest(altblock_finalization_test_2 altblock_finalization_test_2.cpp)
# addtest(altblock_finalization_test_3 altblock_finalization_test_3.cpp)
# addtest(vbkblock_finalization_test vbkblock_finalization_test.cpp)

addtest(finalized_payloads_index_test finalized_payloads_index_test.cpp)
set_test_cost(finalized_payloads_index_test 100000) # 100 sec
