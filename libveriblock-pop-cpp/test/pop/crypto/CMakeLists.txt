add_subdirectory(progpow)

addtest(vblake_test vblake_test.cpp)
addtest(signutil_test secp256k1.cpp)
addtest(progpow_cache_test progpow_cache_test.cpp)
