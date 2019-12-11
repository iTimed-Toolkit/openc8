#ifndef IPWNDFU_REWRITE_C_COMMAND_H
#define IPWNDFU_REWRITE_C_COMMAND_H

#include "checkm8.h"

int dev_memset(struct pwned_device *dev, long addr, unsigned char c, long len);
int dev_memcpy(struct pwned_device *dev, long dest, long src, long len);
int dev_exec(struct pwned_device *dev, long response_len, int nargs, unsigned long long *args);

int dev_read_memory(struct pwned_device *dev, long addr, long len);
int dev_write_memory();

#endif //IPWNDFU_REWRITE_C_COMMAND_H
