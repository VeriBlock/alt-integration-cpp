add_library(rocksdb_storage OBJECT
        rocksdb_impl.cpp
        block_provider_impl.cpp
        )
target_link_libraries(rocksdb_storage
        rocksdb
        )