#add_executable(e2e_fuzz_main main.cpp fuzzer.cpp tree.cpp)
#target_link_libraries(e2e_fuzz_main ${LIB_NAME} pthread)
#target_compile_options(e2e_fuzz_main PUBLIC -fsanitize=address)
#target_link_options(e2e_fuzz_main PUBLIC -fsanitize=address)