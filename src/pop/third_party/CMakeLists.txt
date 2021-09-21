add_library(sha256 OBJECT sha256.cpp)
disable_clang_tidy(sha256)
add_library(secp256k1 OBJECT secp256k1/secp256k1.cpp)
target_add_flag_visibility(secp256k1 PRIVATE -Wno-unused-function)
disable_clang_tidy(secp256k1)


if(WITH_TRACER)
    add_library(tracy OBJECT Tracy.cpp)
    add_compile_definitions(TRACY_ON_DEMAND)
    disable_clang_tidy(tracy)
    target_compile_options(tracy
            PRIVATE
                # disable all warnings from tracy
                -Wno-error
                -w
            )
endif()