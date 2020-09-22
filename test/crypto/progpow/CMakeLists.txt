addtest(progpow_util_test
        progpow_util_test.cpp
        progpow_util_merge_test.cpp
        progpow_util_math_test.cpp
        )
addtest(progpow_test progpow_test.cpp)
addtest(keccakf800_test keccakf800_test.cpp)

addtest(ethash_util_test
        ethash_dag_test.cpp
        ethash_light_test.cpp
        ethash_dagseed_test.cpp
        )
