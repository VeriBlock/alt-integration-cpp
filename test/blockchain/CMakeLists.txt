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

# TODO: fix test
#addtest(alt_blockchain_test
#        alt_blockchain_test.cpp
#        )
#target_link_libraries(alt_blockchain_test
#        ${LIB_NAME}
#        )

addtest(btc_blockchain_test
        btc_blockchain_test.cpp
        # generated cpps
        ${btc_gen_1}
        ${btc_gen_2}
        )
target_link_libraries(btc_blockchain_test
        ${LIB_NAME}
        )
set_target_properties(btc_blockchain_test PROPERTIES
        COST 10000  # 10 sec
        )

addtest(vbk_blockchain_test
        vbk_blockchain_test.cpp
        vbk_blockchain_util_test.cpp
        # generated cpps
        ${vbk_gen_1}
        ${vbk_gen_2}
        )
target_link_libraries(vbk_blockchain_test
        ${LIB_NAME}
        )
set_target_properties(vbk_blockchain_test PROPERTIES
        COST 300000  # many sec
        )

addtest(vbk_getblockproof_test
        vbk_getblockproof_test.cpp
        ${vbk_blocks_gen_1}
        )
target_link_libraries(vbk_getblockproof_test
        ${LIB_NAME}
        )
set_target_properties(vbk_getblockproof_test PROPERTIES
        COST 30000 # 30 sec
        )

addtest(blockchain_test
        chain_test.cpp
        blockchain_test.cpp
        )
target_link_libraries(blockchain_test
        ${LIB_NAME}
        )
set_target_properties(blockchain_test PROPERTIES
        COST 10000  # 10 sec
        )


addtest(chainparams_test chainparams_test.cpp)
target_link_libraries(chainparams_test ${LIB_NAME})