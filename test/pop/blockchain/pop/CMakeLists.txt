addtest(pop_fork_resolution_test pop_fork_resolution_test.cpp)
addtest(pop_vbk_block_tree_test pop_vbk_block_tree_test.cpp)
addtest(pop_vbk_fork_resolution_test pop_vbk_fork_resolution_test.cpp)
set_test_cost(pop_vbk_fork_resolution_test 22500) # 22.5 sec
addtest(pop_context_test pop_context_test.cpp)
addtest(setstate_test setstate_test.cpp)
addtest(vbk_chained_add_payloads_test vbk_chained_add_payloads_test.cpp)
addtest(counting_context_test counting_context_test.cpp)
