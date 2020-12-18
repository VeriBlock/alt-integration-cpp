include(${CMAKE_CURRENT_SOURCE_DIR}/fuzz.cmake)

add_fuzz(popdata_deserialization_fuzz
        SOURCES popdata_deserialization_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/popdata
        )
add_fuzz(altblockindex_deserialization_fuzz
        SOURCES altblockindex_deserialization_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/altblockindex
        )
add_fuzz(btcblockindex_deserialization_fuzz
        SOURCES btcblockindex_deserialization_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/btcblockindex
        )
add_fuzz(vbkblockindex_deserialization_fuzz
        SOURCES vbkblockindex_deserialization_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/vbkblockindex
        )
add_fuzz(popstate_fuzz
        SOURCES popstate_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/popstate
        )
add_fuzz(deserialization_fuzz
        SOURCES deserialization_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/deserialization
        )