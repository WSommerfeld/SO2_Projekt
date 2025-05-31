cmake_minimum_required(VERSION 3.27)
project(Client)

set(CMAKE_CXX_STANDARD 17)

add_executable(Client main.cpp)
target_link_libraries(Client ws2_32 pthread)
set(CMAKE_EXE_LINKER_FLAGS "-static")