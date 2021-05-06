cmake_minimum_required(VERSION 3.19)
project(large_PMP)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread")

add_executable(large_PMP src/main.cpp src/globals.hpp src/instance.cpp src/instance.hpp src/utils.cpp src/utils.hpp src/solution.cpp src/solution.hpp src/TB.cpp src/TB.hpp src/RSSV.cpp src/RSSV.hpp src/semaphore.hpp)