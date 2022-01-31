add_library(poprewards OBJECT
        default_poprewards_calculator.cpp
        )
enable_clang_tidy(poprewards)
