#ifndef CHECKM8_TOOL_CHECKM8_CONFIG_H
#define CHECKM8_TOOL_CHECKM8_CONFIG_H

#define CHECKM8_LOGGING

//#define WITH_ARDUINO
#define ARDUINO_DEV "/dev/ttyACM0"
#define ARDUINO_BAUD 115200

#define CHECKM8_PLATFORM 8010

void checkm8_debug_indent(const char *format, ...);
void checkm8_debug_block(const char *format, ...);

#endif //CHECKM8_TOOL_CHECKM8_CONFIG_H
