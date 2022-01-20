
add_subdirectory(veriblock)

add_library(veriblock-bfi-cpp ${BUILD}
    $<TARGET_OBJECTS:tezt>
)
