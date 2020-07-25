#ifndef CHECKM8_TOOL_COMMAND_H
#define CHECKM8_TOOL_COMMAND_H

#include "checkm8.h"
#include "dev/types.h"

#define CMD_USB_READ_LIMIT  0xFF0

struct dev_cmd_resp *dev_memset(struct pwned_device *dev, DEV_PTR_T addr, unsigned char c, int len);
struct dev_cmd_resp *dev_memcpy(struct pwned_device *dev, DEV_PTR_T dest, DEV_PTR_T src, int len);
struct dev_cmd_resp *dev_exec(struct pwned_device *dev, int response_len, int nargs, unsigned long long *args);

struct dev_cmd_resp *dev_read_memory(struct pwned_device *dev, DEV_PTR_T addr, int len);
struct dev_cmd_resp *dev_write_memory(struct pwned_device *dev, DEV_PTR_T addr, void *data, int len);

#endif //CHECKM8_TOOL_COMMAND_H
