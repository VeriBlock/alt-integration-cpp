include(${CMAKE_CURRENT_SOURCE_DIR}/fuzz.cmake)

add_fuzz(deserialization_fuzz
        SOURCES deserialization_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/deserialization
        )
