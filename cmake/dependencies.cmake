if(WITH_ROCKSDB)
    add_subdirectory(${CMAKE_SOURCE_DIR}/deps/rocksdb)
endif()

find_package(GTest)
if(NOT GTEST_FOUND)
    message(WARNING "Can not find GTest. Tests will not be built.")
    set(TESTING OFF)
endif()
