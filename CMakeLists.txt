cmake_minimum_required(VERSION 3.10)
project(ipwndfu_rewrite_c)
enable_language(C)

include_directories(include)
include_directories(checkm8_libusb/src)
add_library(checkm8_libusb
        checkm8_libusb/src/core.c checkm8_libusb/src/descriptor.c checkm8_libusb/src/hotplug.c
        checkm8_libusb/src/io.c checkm8_libusb/src/strerror.c checkm8_libusb/src/sync.c
        checkm8_libusb/src/hotplug.h include/libusb.h checkm8_libusb/src/libusbi.h checkm8_libusb/src/version.h

        checkm8_libusb/src/os/linux_netlink.c checkm8_libusb/src/os/linux_usbfs.c checkm8_libusb/src/os/linux_udev.c
        checkm8_libusb/src/os/threads_posix.c checkm8_libusb/src/os/poll_posix.c
        checkm8_libusb/src/os/linux_usbfs.h checkm8_libusb/src/os/threads_posix.h checkm8_libusb/src/os/poll_posix.h)

add_subdirectory(checkm8_remote)
add_subdirectory(checkm8_payloads)
