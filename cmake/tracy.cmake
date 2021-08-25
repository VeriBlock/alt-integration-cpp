FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy
        GIT_TAG        dfb4020a924e45e44eaaffb952afb822b2b8acea
)

FetchContent_GetProperties(tracy)
if(NOT tracy_POPULATED)
    message(STATUS "Downloading tracy...")

    set(TRACY_ON_DEMAND OFF)
    set(TRACY_CALLSTACK OFF)
    set(TRACY_ONLY_LOCALHOST ON)
    set(TRACY_NO_BROADCAST OFF)
    set(TRACY_NO_CODE_TRANSFER OFF)
    set(TRACY_NO_CONTEXT_SWITCH OFF)
    set(TRACY_NO_EXIT ON)
    FetchContent_Populate(tracy)
    add_compile_options(-DTRACY_ENABLE=1)
    add_library(tracy
            ${tracy_SOURCE_DIR}/TracyClient.cpp
            ${tracy_SOURCE_DIR}/Tracy.hpp
            )
    find_package(Threads REQUIRED)
    target_link_libraries(tracy Threads::Threads)
    if(UNIX)
        target_link_libraries(tracy dl)
    else()
        message(FATAL_ERROR "Tracy requires library dl. Come here and fix if possible.")
    endif()
    target_compile_options(tracy
            PRIVATE -Wno-unused-parameter -Wno-sometimes-uninitialized
            )
    include_directories(${tracy_SOURCE_DIR})
    disable_clang_tidy(tracy)
endif()
