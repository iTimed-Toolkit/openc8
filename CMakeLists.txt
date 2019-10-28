cmake_minimum_required(VERSION 3.10)
project(ipwndfu_rewrite_c C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -g)

add_executable(ipwndfu exploit/main.c exploit/libusb_helpers.c exploit/libusb_helpers.h)
target_link_libraries(ipwndfu usb-1.0)
