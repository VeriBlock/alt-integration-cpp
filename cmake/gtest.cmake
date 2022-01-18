FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.10.0
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
)

 # Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON)
set(BUILD_GMOCK OFF)
set(BUILD_GTEST ON)
set(INSTALL_GTEST OFF)

FetchContent_MakeAvailable(googletest)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using gcc
    target_compile_options(gtest PRIVATE
        -Wno-maybe-uninitialized
    )
endif()
