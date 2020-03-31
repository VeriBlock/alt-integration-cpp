addtest(pop_utils_test
        pop_fork_resolution_test.cpp
        pop_vbk_block_tree_test.cpp
        pop_state_machine_test.cpp
        )
target_link_libraries(pop_utils_test ${LIB_NAME})

addtest(pop_vbk_fork_resolution_test pop_vbk_fork_resolution_test.cpp)
target_link_libraries(pop_vbk_fork_resolution_test ${LIB_NAME})