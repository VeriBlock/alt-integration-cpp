include(${CMAKE_CURRENT_SOURCE_DIR}/fuzz.cmake)

vbk_define(VBK_FUZZING_UNSAFE_FOR_PRODUCTION)

add_fuzz(deserialization_fuzz
        SOURCES deserialization_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/deserialization
        )
add_fuzz(value_sorted_map_fuzz
        SOURCES value_sorted_map_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/value_sorted_map
        )