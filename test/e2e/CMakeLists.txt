add_library(e2e_utils STATIC e2e_utils.cpp)

addtest(e2e_utils_test e2e_utils_test.cpp)
target_link_libraries(e2e_utils_test e2e_utils)