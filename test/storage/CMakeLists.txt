if(WITH_ROCKSDB)
    addtest(blocks_storage_test blocks_storage_test.cpp)
    target_link_libraries(blocks_storage_test rocksdb)
    set_target_properties(blocks_storage_test PROPERTIES
            RUN_SERIAL TRUE
            )

    addtest(alttree_storage_test alttree_storage_test.cpp)
    target_link_libraries(alttree_storage_test rocksdb)
    set_target_properties(alttree_storage_test PROPERTIES
            RUN_SERIAL TRUE
            )
endif()