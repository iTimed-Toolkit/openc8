set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -g)

add_executable(checkm8_remote main.c conf.h
        libusb_helpers.c libusb_helpers.h exploit.c
        checkm8.h commands.c)


target_link_libraries(checkm8_remote libusb_checkm8 pthread udev)