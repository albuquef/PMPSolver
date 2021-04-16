cmake_minimum_required(VERSION 3.19)
project(large_PMP)

set(CMAKE_CXX_STANDARD 20)

add_executable(large_PMP src/main.cpp src/types.hpp src/instance.cpp src/instance.hpp)