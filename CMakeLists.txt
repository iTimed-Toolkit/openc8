cmake_minimum_required(VERSION 3.10)
project(checkm8_tool)
enable_language(C ASM)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

include_directories(c8_remote/include)
include_directories(include)

add_subdirectory(c8_remote/lib)
add_subdirectory(c8_remote)
