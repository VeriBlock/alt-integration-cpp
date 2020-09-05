include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)


add_library(veriblock-pop-c
        impl/pop_context.cpp
        impl/config.cpp
        )
target_link_libraries(veriblock-pop-c PRIVATE ${LIB_NAME})
