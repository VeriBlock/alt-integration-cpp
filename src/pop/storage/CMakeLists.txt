set(STORAGE_SOURCES
        payloads_index.cpp
        util.cpp
        payloads_provider.cpp
        stored_btc_block_addon.cpp
        stored_vbk_block_addon.cpp
        stored_alt_block_addon.cpp
        adaptors/block_provider_impl.cpp)

if(WITH_ROCKSDB)
        list(APPEND STORAGE_SOURCES adaptors/rocksdb_impl.cpp)
endif()

if(WITH_LEVELDB)
        list(APPEND STORAGE_SOURCES adaptors/leveldb_impl.cpp)
endif()

add_library(storage OBJECT ${STORAGE_SOURCES})

if(WITH_ROCKSDB)
        target_link_libraries(storage rocksdb)
endif()

if(WITH_LEVELDB)
        target_link_libraries(storage leveldb)
endif()