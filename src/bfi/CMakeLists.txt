add_subdirectory(veriblock)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_executable(asio_server
    asio-server.cpp
)
target_link_libraries(asio_server
    Boost::boost
    Threads::Threads
)
