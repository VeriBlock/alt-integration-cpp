addtest(alttree_storage_test alttree_storage_test.cpp)

addtest(save_load_tree_test save_load_tree_test.cpp)

if(WITH_ROCKSDB)
    addtest(blocks_storage_test blocks_storage_test.cpp)
    target_link_libraries(blocks_storage_test rocksdb)
    set_tests_properties(blocks_storage_test PROPERTIES
            RUN_SERIAL TRUE
            COST 6000 # 6 sec
            )

    target_link_libraries(alttree_storage_test rocksdb)
    set_tests_properties(alttree_storage_test PROPERTIES
            RUN_SERIAL TRUE
            )
endif()