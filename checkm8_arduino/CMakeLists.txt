cmake_minimum_required(VERSION 3.10)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(CMAKE_CXX_STANDARD 98)
set(PROJECT_NAME checkm8_arduino)

set(${PROJECT_NAME}_BOARD uno)
project(${PROJECT_NAME})

set(${PROJECT_NAME}_HDRS include/User_Setup.h)
set(${PROJECT_NAME}_SKETCH src/checkm8_arduino.ino)
set(${PROJECT_NAME}_PROGRAMMER avrispmkii)
set(${PROJECT_NAME}_PORT /dev/ttyACM0)

generate_arduino_firmware(${PROJECT_NAME})