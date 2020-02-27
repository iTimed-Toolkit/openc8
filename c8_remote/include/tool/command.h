#ifndef CHECKM8_TOOL_COMMAND_H
#define CHECKM8_TOOL_COMMAND_H

#include "checkm8.h"

#define CMD_USB_READ_LIMIT  0xFF0

struct dev_cmd_resp *dev_memset(struct pwned_device *dev, unsigned long long addr, unsigned char c, int len);
struct dev_cmd_resp *dev_memcpy(struct pwned_device *dev, unsigned long long dest, unsigned long long src, int len);
struct dev_cmd_resp *dev_exec(struct pwned_device *dev, int response_len, int nargs, unsigned long long *args);

struct dev_cmd_resp *dev_read_memory(struct pwned_device *dev, unsigned long long addr, int len);
struct dev_cmd_resp *dev_write_memory(struct pwned_device *dev, unsigned long long addr, unsigned char *data, int len);

#endif //CHECKM8_TOOL_COMMAND_H
