add_library(veriblock_pop_c
        alt_block_tree.cpp
        config.cpp
        )
target_link_libraries(veriblock_pop_c PRIVATE ${LIB_NAME})
