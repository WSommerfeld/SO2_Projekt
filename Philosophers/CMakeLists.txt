cmake_minimum_required(VERSION 3.27)
project(Filozofowie)

set(CMAKE_CXX_STANDARD 14)

add_executable(Filozofowie
        main.cpp)
set(CMAKE_EXE_LINKER_FLAGS "-static")