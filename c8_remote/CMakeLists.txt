cmake_minimum_required(VERSION 3.10)
project(checkm8_remote C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-g -Wall")

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/lib/include)
add_subdirectory(lib)

add_executable(checkm8_remote main.c src/usb_helpers.c src/exploit.c src/payload.c src/command.c)
target_link_libraries(checkm8_remote usb-1.0 pthread udev m)
target_link_libraries(checkm8_remote payload experiments host_crypto)