include(${CMAKE_CURRENT_SOURCE_DIR}/fuzz.cmake)

add_fuzz(popdata_deserialization_fuzz
        SOURCES popdata_deserialization_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/popdata
        )
