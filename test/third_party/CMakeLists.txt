addtest(sha256_test sha256_test.cpp)
target_link_libraries(sha256_test ${LIB_NAME})

addtest(secp256k1_test secp256k1_test.cpp)
target_link_libraries(secp256k1_test ${LIB_NAME})

addtest(gmock_test gmock_test.cpp)
