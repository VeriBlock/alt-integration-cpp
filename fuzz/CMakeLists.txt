include(${CMAKE_CURRENT_SOURCE_DIR}/fuzz.cmake)

vbk_define(VBK_FUZZING_UNSAFE_FOR_PRODUCTION)
# if you add new fuzzer, bump this number
set(TOTAL_FUZZERS 3)


add_subdirectory(e2e)


add_fuzz(deserialization_fuzz
        SOURCES deserialization_fuzz.cpp
        )
add_fuzz(value_sorted_map_fuzz
        SOURCES value_sorted_map_fuzz.cpp
        )

add_fuzz(e2e_fuzz
        SOURCES
            e2e/fuzzer.cpp
            e2e/tree.cpp
            e2e/tree.hpp
        )
