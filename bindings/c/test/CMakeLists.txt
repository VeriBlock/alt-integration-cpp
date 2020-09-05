link_libraries(veriblock-pop-c)

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

add_executable(veriblock-pop-c-test
        $<TARGET_OBJECTS:veriblock-pop-c>
        test_runner.c
        extern_test.c
        config_test.c
        )