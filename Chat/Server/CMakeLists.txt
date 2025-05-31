cmake_minimum_required(VERSION 3.27)
project(Server)

set(CMAKE_CXX_STANDARD 17)

add_executable(Server main.cpp)
add_executable(erver main.cpp)
target_link_libraries(Server ws2_32 pthread stdc++)
set(CMAKE_EXE_LINKER_FLAGS "-static")

