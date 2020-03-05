include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(storage)
add_subdirectory(blockchain)
add_subdirectory(third_party)
add_subdirectory(entities)

addtest(popmanager_test popmanager_test.cpp)
target_link_libraries(popmanager_test vbkutils)

addtest(vblake_test vblake_test.cpp)
target_link_libraries(vblake_test vbkutils)

addtest(streamutils_test streamutils_test.cpp)
target_link_libraries(streamutils_test vbkutils)

addtest(base58_test base58_test.cpp)
target_link_libraries(base58_test vbkutils)

addtest(base59_test base59_test.cpp)
target_link_libraries(base59_test vbkutils)

addtest(stateless_validation_test stateless_validation_test.cpp)
target_link_libraries(stateless_validation_test vbkutils)

addtest(arith_uint256_test arith_uint256_test.cpp)
target_link_libraries(arith_uint256_test vbkutils)

addtest(signutil_test signutil_test.cpp)
target_link_libraries(signutil_test vbkutils)

addtest(mock_miner_test mock_miner_test.cpp)
target_link_libraries(mock_miner_test vbkutils)

addtest(keystone_util_test keystone_util_test.cpp)
target_link_libraries(keystone_util_test vbkutils)


if(WITH_ROCKSDB)
    addtest(state_manager_test state_manager_test.cpp)
    target_link_libraries(state_manager_test vbkutils rocksdb)
endif()