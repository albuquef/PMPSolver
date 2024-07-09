cmake_minimum_required(VERSION 3.10)
project(large_PMP)

# Set default value for USE_CLUSTER if not provided by the user
if (NOT DEFINED USE_CLUSTER)
    set(USE_CLUSTER OFF CACHE BOOL "Use cluster" FORCE)
endif()

# CPLEX_ROOT_DIR - path to CPLEX installation
if (USE_CLUSTER)
    set(CPLEX_ROOT_DIR "/usr/local/ibm/ILOG/CPLEX_Studio1210")
else()
    set(CPLEX_ROOT_DIR "/opt/ibm/ILOG/CPLEX_Studio2211")
endif(USE_CLUSTER)

# CXX_FLAGS - compiler flags for C++ and cplex
set(CMAKE_CXX_STANDARD 17) # C++17 same as cluster
set(CMAKE_CXX_FLAGS "-O3 -pthread -fopenmp -Wall -Wextra -std=c++17")
# Uncomment the following line if you want to set CMAKE_CXX_STANDARD globally
# set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Include directories and search CPLEX library
include_directories("${CPLEX_ROOT_DIR}/include")
# find_package(Cplex REQUIRED)
message("USE_CLUSTER: ${USE_CLUSTER}")

find_package(Cplex REQUIRED)
# message("CPLEX_FOUND: ${CPLEX_FOUND}")
# message("CPLEX_ROOT_DIR: ${CPLEX_ROOT_DIR}")
# message("CPLEX_INCLUDE_DIR: ${CPLEX_INCLUDE_DIR}")
# message("CPLEX_LIBRARY: ${CPLEX_LIBRARY}")
# message("CPLEX_ILOCPLEX_LIBRARY: ${CPLEX_ILOCPLEX_LIBRARY}")
# message("CPLEX_CONCERT_LIBRARY: ${CPLEX_CONCERT_LIBRARY}")
# CPLEX_COMPILER_FLAGS is assumed to be set by the find_package(Cplex REQUIRED) command
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPLEX_COMPILER_FLAGS}")

INCLUDE_DIRECTORIES(${CPLEX_INCLUDE_DIRS})

MESSAGE("LOG")
MESSAGE("${SRCS}")

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
    src/PMP.hpp src/PMP.cpp
    src/VNS.hpp src/VNS.cpp
    src/PostOptimization.hpp src/PostOptimization.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CPLEX_LIBRARIES})

if (USE_CLUSTER)
    # Set the path to CPLEX libraries
    set(CPLEX_LIB_DIR "/usr/local/ibm/ILOG/CPLEX_Studio1210/cplex/lib/x86-64_linux/static_pic")
    # Add CPLEX flags
    target_link_libraries(${PROJECT_NAME}
        ${CPLEX_LIB_DIR}/libcplexdistmip.a
        ${CPLEX_LIB_DIR}/libilocplex.a
        ${CPLEX_LIB_DIR}/libcplex.a
        -ldl
    )
else()
    # Set the path to CPLEX libraries
    set(CPLEX_LIB_DIR "/opt/ibm/ILOG/CPLEX_Studio2211/cplex/lib/x86-64_linux/static_pic")
    # Add CPLEX flags
    target_link_libraries(${PROJECT_NAME}
        ${CPLEX_LIB_DIR}/libilocplex.a
        ${CPLEX_LIB_DIR}/libcplex.a
        -ldl
    )
endif(USE_CLUSTER)
