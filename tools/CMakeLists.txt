add_executable(filetoheader EXCLUDE_FROM_ALL filetoheader.cpp)
add_executable(build_payouts_table EXCLUDE_FROM_ALL build_payouts_table.cpp)
target_link_libraries(build_payouts_table ${LIB_NAME})

add_executable(build_seedhash_epochs build_seedhash_epochs.cpp)
target_link_libraries(build_seedhash_epochs ${LIB_NAME})
