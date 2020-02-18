addtest(storage_test storage_test.cpp)
target_link_libraries(storage_test vbkutils)

if(WITH_ROCKSDB)
    target_link_libraries(storage_test rocksdb)
    addtest(rocks_storage_test rocks_storage_test.cpp)
    target_link_libraries(rocks_storage_test vbkutils rocksdb)
endif()