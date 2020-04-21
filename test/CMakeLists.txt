link_libraries(${LIB_NAME})
include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

# TODO: remove storage tests, as we are not using them now
#add_subdirectory(storage)
add_subdirectory(e2e)
add_subdirectory(blockchain)
add_subdirectory(rewards)
add_subdirectory(third_party)
add_subdirectory(entities)

addtest(vblake_test vblake_test.cpp)

addtest(streamutils_test streamutils_test.cpp)

addtest(base58_test base58_test.cpp)

addtest(base59_test base59_test.cpp)


gencpp(
        btcblockheaders
        config_btcblocks.txt
        btc_headers
)
gencpp(
        vbkblockheaders
        config_vbkblocks.txt
        vbk_headers
)
addtest(config_test config_test.cpp
        ${btc_headers}
        ${vbk_headers}
        )

addtest(stateless_validation_test stateless_validation_test.cpp)

addtest(arith_uint256_test arith_uint256_test.cpp)

addtest(signutil_test signutil_test.cpp)

addtest(keystone_util_test keystone_util_test.cpp)

addtest(validationstate_test validationstate_test.cpp)

addtest(alt-util_test alt-util_test.cpp)
