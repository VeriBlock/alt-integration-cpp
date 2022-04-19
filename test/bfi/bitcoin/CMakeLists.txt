add_subdirectory(net)

addtest(serialize_test serialize_test.cpp)
addtest(btc_block_test block_test.cpp)
addtest(btc_transaction_test transaction_test.cpp)