if(TESTING AND TESTING_GO)
    find_program(GO go REQUIRED)
    add_test(
            NAME go_test
            COMMAND go test ./...
            WORKING_DIRECTORY
    )
    set_tests_properties(go_test PROPERTIES
            ENVIRONMENT
                VBK_INCLUDE_DIR=${CMAKE_SOURCE_DIR}/include
            )
endif()
