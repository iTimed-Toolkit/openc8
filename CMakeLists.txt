cmake_minimum_required(VERSION 3.10)
project(ipwndfu_rewrite_c C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -g)

add_executable(ipwndfu main.c libusb_helpers.c libusb_helpers.h)
target_link_libraries(ipwndfu usb-1.0)
