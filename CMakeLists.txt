cmake_minimum_required(VERSION 3.10)
project(checkm8_tool)
enable_language(C ASM)

include_directories(include)

# set up checkm8_remote
add_subdirectory(c8_remote)
