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
target_link_libraries(vblake_test ${LIB_NAME})

addtest(streamutils_test streamutils_test.cpp)
target_link_libraries(streamutils_test ${LIB_NAME})

addtest(base58_test base58_test.cpp)
target_link_libraries(base58_test ${LIB_NAME})

addtest(base59_test base59_test.cpp)
target_link_libraries(base59_test ${LIB_NAME})

addtest(config_test config_test.cpp)
target_link_libraries(config_test ${LIB_NAME})

addtest(stateless_validation_test stateless_validation_test.cpp)
target_link_libraries(stateless_validation_test ${LIB_NAME})

addtest(arith_uint256_test arith_uint256_test.cpp)
target_link_libraries(arith_uint256_test ${LIB_NAME})

addtest(signutil_test signutil_test.cpp)
target_link_libraries(signutil_test ${LIB_NAME})

addtest(keystone_util_test keystone_util_test.cpp)
target_link_libraries(keystone_util_test ${LIB_NAME})

addtest(validationstate_test validationstate_test.cpp)
target_link_libraries(validationstate_test ${LIB_NAME})

addtest(alt-util_test alt-util_test.cpp)
target_link_libraries(alt-util_test ${LIB_NAME})