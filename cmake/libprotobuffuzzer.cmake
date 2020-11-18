#FetchContent_Declare(
#        libprotobuffuzzer
#        GIT_REPOSITORY https://github.com/google/libprotobuf-mutator
#        GIT_TAG        v1.0
#)
#
#FetchContent_GetProperties(libprotobuffuzzer)
#if(NOT libprotobuffuzzer_POPULATED)
#    message(STATUS "Downloading libprotobuffuzzer...")
#
#    set(LIB_PROTO_MUTATOR_TESTING OFF)
#    set(LIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF ON)
#    FetchContent_Populate(libprotobuffuzzer)
#    add_subdirectory(${libprotobuffuzzer_SOURCE_DIR} ${libprotobuffuzzer_BINARY_DIR} EXCLUDE_FROM_ALL)
#endif()
