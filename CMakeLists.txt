cmake_minimum_required(VERSION 3.20)
project(heartbeat)

set(CMAKE_CXX_STANDARD 17)

add_executable(server server.cpp)
add_executable(client client.cpp)
target_link_libraries(server boost_system pthread )
target_link_libraries(client boost_system pthread )
#add_executable(heartbeat main.cpp)
