#ifndef CHECKM8_TOOL_COMMAND_H
#define CHECKM8_TOOL_COMMAND_H

#include "checkm8.h"
#include "dev/shared_types.h"

#define CMD_USB_READ_LIMIT  0xFF0

int reset_img_base(struct pwned_device *dev);
int command(struct pwned_device *dev,
            short intf,
            void *cmd_args, int cmd_arg_len,
            void *resp_buf, int response_len);

#endif //CHECKM8_TOOL_COMMAND_H
