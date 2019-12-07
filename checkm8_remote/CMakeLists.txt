set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS -g)

include_directories(include)
add_executable(checkm8_remote main.c src/libusb_helpers.c src/commands.c src/exploit.c)

target_link_libraries(checkm8_remote checkm8_libusb pthread udev)