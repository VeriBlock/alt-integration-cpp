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
        vbkpoptx_test.cpp
        atv_test.cpp
        vtb_test.cpp
        altblock_test.cpp
        popdata_test.cpp
        blockindex_test.cpp
        pop_payouts_test.cpp
        )

addtest(merkle_tree_test merkle_tree_test.cpp)
addtest(json_test
        json_test.cpp
        )