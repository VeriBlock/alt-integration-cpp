if(WITH_ROCKSDB)
add_library(rocksdb_storage OBJECT
        rocksdb_impl.cpp
        block_provider_impl.cpp
        )
target_link_libraries(rocksdb_storage
        rocksdb
        )
endif()

if(WITH_LEVELDB)
add_library(leveldb_storage OBJECT
        leveldb_impl.cpp
        block_provider_impl.cpp
        )
target_link_libraries(leveldb_storage
        leveldb
        )
endif()