cmake_minimum_required(VERSION 3.16)
project(large_PMP)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread -fopenmp")

add_executable(large_PMP src/main.cpp src/globals.hpp src/globals.cpp src/instance.cpp src/instance.hpp src/utils.cpp src/utils.hpp src/solution_std.cpp src/solution_std.hpp src/TB.cpp src/TB.hpp src/RSSV.cpp src/RSSV.hpp src/semaphore.hpp src/solution_cap.cpp src/solution_cap.hpp src/solution.cpp src/solution.hpp src/config_parser.cpp src/toml.hpp src/TBPercentage.cpp src/TBPercentage.hpp)
