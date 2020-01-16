addtest(entities_serde_test
        address_test.cpp
        output_test.cpp
        coin_test.cpp
        btcblock_test.cpp
        merkle_path_test.cpp
        vbk_merkle_path_test.cpp
        vbkblock_test.cpp
        publication_data_test.cpp
        vbktx_test.cpp
        )
target_link_libraries(entities_serde_test vbkutils)