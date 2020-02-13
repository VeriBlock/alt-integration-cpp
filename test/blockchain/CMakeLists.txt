gencpp(
        btc_blockheaders_mainnet_0_10000
        ./btc_blockheaders_mainnet_0_10000
        outcpp1
)
gencpp(
        btc_blockheaders_mainnet_30000_40000
        ./btc_blockheaders_mainnet_30000_40000
        outcpp2
)

addtest(blockchain_test
        chain_test.cpp
        btc_blockchain_test.cpp
        blockchain_test.cpp
        # generated cpps
        ${outcpp1}
        ${outcpp2}
        )
target_link_libraries(blockchain_test vbkutils)


