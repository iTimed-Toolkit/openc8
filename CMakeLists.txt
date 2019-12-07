cmake_minimum_required(VERSION 3.10)
project(ipwndfu_rewrite_c)
enable_language(C)

add_library(checkm8_libusb
        checkm8_remote/conf.h libusb/config.h

        libusb/src/core.c libusb/src/descriptor.c libusb/src/hotplug.c
        libusb/src/io.c libusb/src/strerror.c libusb/src/sync.c
        libusb/src/hotplug.h libusb/src/libusb.h libusb/src/libusbi.h libusb/src/version.h

        libusb/src/os/linux_netlink.c libusb/src/os/linux_usbfs.c libusb/src/os/linux_udev.c
        libusb/src/os/threads_posix.c libusb/src/os/poll_posix.c
        libusb/src/os/linux_usbfs.h libusb/src/os/threads_posix.h libusb/src/os/poll_posix.h)

add_subdirectory(checkm8_remote)
add_subdirectory(checkm8_payload)
