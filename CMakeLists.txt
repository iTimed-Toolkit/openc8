cmake_minimum_required(VERSION 3.10)
project(ipwndfu_rewrite_c)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -g)

add_executable(ipwndfu main.c conf.h
        exploit/libusb_helpers.c exploit/libusb_helpers.h
        exploit/exploit.c exploit/checkm8.h exploit/commands.c)

add_library(libusb_checkm8
        conf.h libusb/config.h

        libusb/libusb/core.c libusb/libusb/descriptor.c libusb/libusb/hotplug.c
        libusb/libusb/io.c libusb/libusb/strerror.c libusb/libusb/sync.c
        libusb/libusb/hotplug.h libusb/libusb/libusb.h libusb/libusb/libusbi.h libusb/libusb/version.h

        libusb/libusb/os/linux_netlink.c libusb/libusb/os/linux_usbfs.c libusb/libusb/os/linux_udev.c
        libusb/libusb/os/threads_posix.c libusb/libusb/os/poll_posix.c
        libusb/libusb/os/linux_usbfs.h libusb/libusb/os/threads_posix.h libusb/libusb/os/poll_posix.h)

target_link_libraries(ipwndfu libusb_checkm8 pthread udev)
