#ifndef CHECKM8_TOOL_EXPERIMENTS_H
#define CHECKM8_TOOL_EXPERIMENTS_H

#include "payload.h"
#include "bootrom_type.h"

/* AES Software */
DEV_PTR_T setup_bern_exp(struct pwned_device *dev);
struct bern_data *get_bern_exp_data(struct pwned_device *dev, DEV_PTR_T async_buf);

DEV_PTR_T setup_corr_exp(struct pwned_device *dev, unsigned char *init_key);
struct corr_data *get_corr_exp_data(struct pwned_device *dev, DEV_PTR_T async_buf);

/* System */
void usb_task_exit(struct pwned_device *dev);

/* Power */
void floppysleep(struct pwned_device *dev);
void floppysleep_async(struct pwned_device *dev);

#endif //CHECKM8_TOOL_EXPERIMENTS_H
