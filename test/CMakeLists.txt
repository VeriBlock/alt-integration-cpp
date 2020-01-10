addtest(vblake_test vblake_test.cpp)
target_link_libraries(vblake_test vblake vbkutils)

addtest(streamutils_test streamutils_test.cpp)
target_link_libraries(streamutils_test vbkutils)

addtest(serde_test serde_test.cpp)
target_link_libraries(serde_test vbkutils)

addtest(base58_test base58_test.cpp)
target_link_libraries(base58_test vbkutils)