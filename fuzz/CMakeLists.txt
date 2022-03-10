include(${CMAKE_CURRENT_SOURCE_DIR}/fuzz.cmake)

vbk_define(VBK_FUZZING_UNSAFE_FOR_PRODUCTION)
# if you add new fuzzer, bump this number with a WEIGHT value.
# WEIGHT defines how "important" fuzzer is. More important fuzzers will run longer in CI.
set(TOTAL_WEIGHT 27)

add_subdirectory(e2e)

add_fuzz(address_deserialization_fuzz
        SOURCES address_deserialization_fuzz.cpp
        ARGS
            -malloc_limit_mb=1
            -rss_limit_mb=1
            -max_len=25
        WEIGHT 3
        )
add_fuzz(deserialization_fuzz
        SOURCES deserialization_fuzz.cpp
        WEIGHT 3
        )
add_fuzz(value_sorted_map_fuzz
        SOURCES value_sorted_map_fuzz.cpp
        WEIGHT 1
        )

add_fuzz(e2e_fuzz
        SOURCES
            e2e/fuzzer.cpp
            e2e/tree.cpp
            e2e/tree.hpp
        ARGS
            # fail if malloc(>10MB) is called
            -malloc_limit_mb=10
            # fail if more than 4 GB of RAM is used
            -rss_limit_mb=4000
            # make maximum length of an input sequence big
            -max_len=100000
        WEIGHT 20
        )
