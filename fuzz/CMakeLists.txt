include(${CMAKE_CURRENT_SOURCE_DIR}/fuzz.cmake)

add_fuzz(popstate_fuzz
        SOURCES popstate_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/popstate
        )
add_fuzz(deserialization_fuzz
        SOURCES deserialization_fuzz.cpp
        CORPUS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/corpus/deserialization
        )