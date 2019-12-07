cmake_minimum_required(VERSION 3.10)
project(ipwndfu_rewrite_c)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -g)

add_executable(ipwndfu main.c conf.h
        exploit/libusb_helpers.c exploit/libusb_helpers.h
        exploit/exploit.c exploit/checkm8.h exploit/commands.c)

add_library(libusb_checkm8
        conf.h libusb/config.h

        libusb/src/core.c libusb/src/descriptor.c libusb/src/hotplug.c
        libusb/src/io.c libusb/src/strerror.c libusb/src/sync.c
        libusb/src/hotplug.h libusb/src/libusb.h libusb/src/libusbi.h libusb/src/version.h

        libusb/src/os/linux_netlink.c libusb/src/os/linux_usbfs.c libusb/src/os/linux_udev.c
        libusb/src/os/threads_posix.c libusb/src/os/poll_posix.c
        libusb/src/os/linux_usbfs.h libusb/src/os/threads_posix.h libusb/src/os/poll_posix.h)

target_link_libraries(ipwndfu libusb_checkm8 pthread udev)
