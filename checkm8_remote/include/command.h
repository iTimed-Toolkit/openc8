#ifndef IPWNDFU_REWRITE_C_COMMAND_H
#define IPWNDFU_REWRITE_C_COMMAND_H

#include "checkm8.h"

int dev_memset(struct pwned_device *dev, long addr, char c, long len);
int dev_memcpy(struct pwned_device *dev, long dest, long src, long len);
int dev_exec(struct pwned_device *dev, long response_len, int nargs, long *args);

int dev_read_memory();
int dev_write_memory();

#endif //IPWNDFU_REWRITE_C_COMMAND_H
