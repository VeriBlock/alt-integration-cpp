link_libraries(${LIB_NAME})
include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_BINARY_DIR}/include)

add_library(test_main OBJECT test_main.cpp)
target_link_libraries(test_main PUBLIC gtest)

add_subdirectory(pop)
add_subdirectory(bfi)
add_subdirectory(e2e)
