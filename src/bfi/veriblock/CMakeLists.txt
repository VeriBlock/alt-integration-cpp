add_library(bfi_veriblock OBJECT transaction.cpp block.cpp)

target_add_protobuf(bfi_veriblock
    veriblock.proto
)
