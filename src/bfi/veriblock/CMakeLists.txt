
add_library(tezt OBJECT tezt.cpp)
target_add_protobuf(tezt
    veriblock.proto
)
