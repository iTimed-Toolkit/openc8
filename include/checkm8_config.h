#ifndef CHECKM8_TOOL_CHECKM8_CONFIG_H
#define CHECKM8_TOOL_CHECKM8_CONFIG_H

//#define LIBUSB_LOGGING
//#define CHECKM8_LOGGING

#define WITH_ARDUINO
#define ARDUINO_DEV "/dev/ttyACM0"
#define ARDUINO_BAUD 115200

#define CHECKM8_PLATFORM 8010
#define CHECKM8_BIN_BASE "/home/grg/Projects/School/NCSU/iphone_aes_sc/checkm8_tool/c8_remote/bin/"

void checkm8_debug_indent(const char *format, ...);
void checkm8_debug_block(const char *format, ...);

#endif //CHECKM8_TOOL_CHECKM8_CONFIG_H
