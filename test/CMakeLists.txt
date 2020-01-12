include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(entities)

addtest(vblake_test vblake_test.cpp)
target_link_libraries(vblake_test vblake vbkutils)

addtest(streamutils_test streamutils_test.cpp)
target_link_libraries(streamutils_test vbkutils)

addtest(base58_test base58_test.cpp)
target_link_libraries(base58_test vbkutils)

addtest(base59_test base59_test.cpp)
target_link_libraries(base59_test vbkutils)