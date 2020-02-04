if(WIN32)
    find_package(RocksDB CONFIG REQUIRED)
endif()

if(NOT WIN32)
    find_package(RocksDB CONFIG REQUIRED)
endif()

find_package(GTest)
if(NOT GTEST_FOUND)
    message(WARNING "Can not find GTest. Tests will not be built.")
    set(TESTING OFF)
endif()
