if(TESTING AND TESTING_GO)
    if(NOT WITH_C_WRAPPER)
        message(FATAL_ERROR "Please build library with the c wrapper, -DWTIH_C_WRAPPER=ON")
    endif()

    find_program(GO go REQUIRED)
    add_test(
            NAME go_test
            COMMAND go test ./...
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()
