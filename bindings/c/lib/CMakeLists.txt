add_library(veriblock_pop_c
        alt_block_tree.cpp
        payloads_provider.cpp
        config.cpp
        )
target_link_libraries(veriblock_pop_c PRIVATE ${LIB_NAME})

