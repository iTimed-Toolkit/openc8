cmake_minimum_required(VERSION 3.10)
project(tools C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-g -Wall -fopenmp")

include_directories(include)

add_executable(tool_strip  correlation/strip/main.c)
add_executable(tool_analyze correlation/analyze/main.c)
add_executable(tool_verify correlation/verify/main.c)

target_link_libraries(tool_analyze host_crypto mpi m)
target_link_libraries(tool_verify host_crypto m)