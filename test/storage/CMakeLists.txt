if(WITH_ROCKSDB)
    addtest(blocks_storage_test blocks_storage_test.cpp)
    target_link_libraries(blocks_storage_test rocksdb)

    addtest(alttree_storage_test alttree_storage_test.cpp)
    target_link_libraries(alttree_storage_test rocksdb)
endif()