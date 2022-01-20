# create ct_params.hpp
configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/ct_params.hpp.in
    ${PROJECT_SOURCE_DIR}/libveriblock-pop-cpp/include/veriblock/pop/ct_params.hpp
)

add_subdirectory(pop)
