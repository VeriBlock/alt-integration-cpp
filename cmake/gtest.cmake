FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.10.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    message(STATUS "Downloading googletest...")

    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using gcc
        target_compile_options(gtest PRIVATE
                -Wno-maybe-uninitialized
                )
    endif()
    disable_clang_tidy(gtest)
    disable_clang_tidy(gtest_main)
    disable_clang_tidy(gmock)
    disable_clang_tidy(gmock_main)
endif()
