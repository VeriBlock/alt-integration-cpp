gencpp(
        btc_blockheaders_mainnet_0_10000
        btc_blockheaders_mainnet_0_10000.txt
        outcpp1
)
gencpp(
        btc_blockheaders_mainnet_30000_40000
        btc_blockheaders_mainnet_30000_40000.txt
        outcpp2
)

gencpp(
        btc_blockheaders_testnet_0_10000
        btc_blockheaders_testnet_0_10000.txt
        outcpp3
)

addtest(btc_blockchain_test
        btc_blockchain_test.cpp
        # generated cpps
        ${outcpp1}
        ${outcpp2}
        ${outcpp3}
        )
target_link_libraries(btc_blockchain_test
        vbkutils
        )
        
addtest(vbk_blockchain_test
        vbk_blockchain_util_test.cpp)

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


