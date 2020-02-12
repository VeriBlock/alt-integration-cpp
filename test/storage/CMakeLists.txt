#if(WITH_ROCKSDB)
#    addtest(rocks_storage_test rocks_storage_test.cpp)
#    target_link_libraries(rocks_storage_test vbkutils)
#endif()

addtest(inmem_storage_test inmem_storage_test.cpp)
target_link_libraries(inmem_storage_test vbkutils)