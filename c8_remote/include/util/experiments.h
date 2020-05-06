#ifndef CHECKM8_TOOL_EXPERIMENTS_H
#define CHECKM8_TOOL_EXPERIMENTS_H

#include "tool/payload.h"
#include "dev/types.h"

/* AES Software */
DEV_PTR_T setup_bern_exp(struct pwned_device *dev, unsigned char key[16], unsigned int num_iter, unsigned int offset);
struct bern_data *get_bern_exp_data(struct pwned_device *dev, DEV_PTR_T async_buf);

/* System */
void usb_task_exit(struct pwned_device *dev);

/* Power */
void floppysleep(struct pwned_device *dev);
void floppysleep_async(struct pwned_device *dev);

#endif //CHECKM8_TOOL_EXPERIMENTS_H
