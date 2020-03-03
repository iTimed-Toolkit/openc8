cmake_minimum_required(VERSION 3.10)
project(tools C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-g -Wall -fopenmp")

include_directories(include)

add_executable(strip  correlation/strip/main.c)
add_executable(analyze correlation/analyze/main.c)
target_link_libraries(analyze host_crypto mpi m)