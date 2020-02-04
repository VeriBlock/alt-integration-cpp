if(WIN32)
    find_package(RocksDB CONFIG REQUIRED)
else()
    find_package(RocksDB REQUIRED)
endif()

find_package(GTest)
if(NOT GTEST_FOUND)
    message(WARNING "Can not find GTest. Tests will not be built.")
    set(TESTING OFF)
endif()
