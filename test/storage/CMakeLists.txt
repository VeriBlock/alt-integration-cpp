addtest(alttree_storage_test alttree_storage_test.cpp)

if(WITH_ROCKSDB)
    include_directories(${CMAKE_BINARY_DIR}/rocksdb-src/include)

    addtest(storage_test storage_test.cpp)
    target_link_libraries(storage_test rocksdb)

    addtest(rocks_storage_test rocks_storage_test.cpp)
    target_link_libraries(rocks_storage_test rocksdb)

    addtest(endorsement_storage_test endorsement_storage_test.cpp)
    target_link_libraries(endorsement_storage_test rocksdb)
            
    addtest(payloads_storage_test payloads_storage_test.cpp)
    target_link_libraries(payloads_storage_test rocksdb)
endif()