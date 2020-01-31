# target_link_libraries(main PRIVATE RocksDB::rocksdb RocksDB::rocksdb-shared)
find_package(RocksDB CONFIG REQUIRED)

find_package(GTest)
if(NOT GTEST_FOUND)
    message(WARNING "Can not find GTest. Tests will not be built.")
    set(TESTING OFF)
endif()
