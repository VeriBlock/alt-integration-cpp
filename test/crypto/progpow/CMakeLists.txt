addtest(progpow_util_test progpow_util_test.cpp)
addtest(progpow_util_merge_test progpow_util_merge_test.cpp)
addtest(progpow_util_math_test progpow_util_math_test.cpp)
addtest(keccakf800_test keccakf800_test.cpp)
addtest(progpow_test progpow_test.cpp)
set_tests_properties(progpow_test PROPERTIES
        COST 9000 # ~9 sec
        )
addtest(ethash_dag_test ethash_dag_test.cpp)
set_tests_properties(ethash_dag_test PROPERTIES
        COST 26000 # ~26 sec
        )
addtest(ethash_light_test ethash_light_test)
set_tests_properties(ethash_light_test PROPERTIES
        COST 6500 # ~6.5 sec
        )
addtest(ethash_dagseed_test ethash_dagseed_test)
set_tests_properties(ethash_dagseed_test PROPERTIES
        COST 10000 # ~10 sec
        )
