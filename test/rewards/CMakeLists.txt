addtest(poprewards_calculator_test
        poprewards_calculator_test.cpp)
target_link_libraries(poprewards_calculator_test ${LIB_NAME})

if(WITH_ROCKSDB)
    addtest(poprewards_test
        poprewards_test.cpp)
    target_link_libraries(poprewards_test ${LIB_NAME} rocksdb)
    set_target_properties(poprewards_test PROPERTIES
            RESOURCE_LOCK db-test)
endif()