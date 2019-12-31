cmake_minimum_required(VERSION 3.10)
project(checkm8_tool)
enable_language(C)

include_directories(include)
include_directories(c8_libusb/src)
include_directories(c8_libusb/src/os)
add_library(checkm8_libusb
        c8_libusb/src/core.c c8_libusb/src/descriptor.c c8_libusb/src/hotplug.c
        c8_libusb/src/io.c c8_libusb/src/strerror.c c8_libusb/src/sync.c

        c8_libusb/src/os/linux_netlink.c c8_libusb/src/os/linux_usbfs.c c8_libusb/src/os/linux_udev.c
        c8_libusb/src/os/threads_posix.c c8_libusb/src/os/poll_posix.c)

add_subdirectory(c8_remote)
add_subdirectory(c8_payloads)
