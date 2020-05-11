add_library(logger OBJECT
        logger.cpp
        logger_default.cpp
        )
target_include_directories(logger PRIVATE ${CMAKE_SOURCE_DIR}/include/veriblock/third_party)