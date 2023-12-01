cmake_minimum_required(VERSION 3.16)
project(large_PMP)


if(NOT "${CMAKE_CXX_STANDARD}")
  set(CMAKE_CXX_STANDARD 20)
endif()

# CPLEX_ROOT_DIR - path to CPLEX installation
set(CPLEX_ROOT_DIR "/opt/ibm/ILOG/CPLEX_Studio2211")

# CXX_FLAGS - compiler flags for C++ and cple
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3 -pthread -fopenmp -Wall -Wextra -std=c++11 -m64 -g -O -fPIC -fexceptions -DNDEBUG -DIL_STD")


set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# search CPLEX library
find_package(Cplex REQUIRED)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPLEX_COMPILER_FLAGS}")
MESSAGE ("${CPLEX_COMPILER_FLAGS}")

INCLUDE_DIRECTORIES( ${CPLEX_INCLUDE_DIRS})


MESSAGE ("LOG")
MESSAGE ("${SRCS}")


set(SOURCE_FILES
    src/main.cpp 
    src/globals.hpp src/globals.cpp 
    src/instance.cpp src/instance.hpp 
    src/utils.cpp src/utils.hpp 
    src/solution_std.cpp src/solution_std.hpp 
    src/TB.cpp src/TB.hpp 
    src/RSSV.cpp src/RSSV.hpp 
    src/semaphore.hpp 
    src/solution_cap.cpp src/solution_cap.hpp 
    src/config_parser.cpp 
    src/toml.hpp 
    src/TBPercentage.hpp src/TBPercentage.cpp
    src/PMP.hpp src/PMP.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${CPLEX_LIBRARIES})
