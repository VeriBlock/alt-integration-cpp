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
        vbk_blockheaders_mainnet_0_10000
        vbk_blockheaders_mainnet_0_10000.txt
        vbk_gen_1
)

gencpp(
        vbk_blockheaders_testnet_0_10000
        vbk_blockheaders_testnet_0_10000.txt
        vbk_gen_2
)


addtest(btc_blockchain_test
        btc_blockchain_test.cpp
        # generated cpps
        ${btc_gen_1}
        ${btc_gen_2}
        )
target_link_libraries(btc_blockchain_test
        vbkutils
        )


addtest(vbk_blockchain_test
        vbk_blockchain_test.cpp
        vbk_blockchain_util_test.cpp
        # generated cpps
        ${vbk_gen_1}
        ${vbk_gen_2}
        )
target_link_libraries(vbk_blockchain_test
        vbkutils
        )


addtest(blockchain_test
        chain_test.cpp
        blockchain_test.cpp
        )
target_link_libraries(blockchain_test
        vbkutils
        )


addtest(chainparams_test chainparams_test.cpp)
target_link_libraries(chainparams_test vbkutils)