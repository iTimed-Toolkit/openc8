#ifndef CHECKM8_TOOL_PAYLOAD_H
#define CHECKM8_TOOL_PAYLOAD_H

#include "checkm8.h"
#include "libpayload.h"
#include "dev/shared_types.h"

int sync_payloads(struct pwned_device *dev);
int sync_data(struct pwned_device *dev);
int install_utils(struct pwned_device *dev);

int install_payload(struct pwned_device *dev, PAYLOAD_T p);
int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p);
int uninstall_all_payloads(struct pwned_device *dev);
int payload_is_installed(struct pwned_device *dev, PAYLOAD_T p);

int execute_payload(struct pwned_device *dev, PAYLOAD_T p, void *arg, int arg_len, void *resp, int resp_len);
int async_payload_create(struct pwned_device *dev, PAYLOAD_T p, DEV_PTR_T arg);
int async_payload_run(struct pwned_device *dev, PAYLOAD_T p);
int async_payload_kill(struct pwned_device *dev, PAYLOAD_T p, DEV_PTR_T buf_addr);

DEV_PTR_T install_data(struct pwned_device *dev, void *data, int len);
int uninstall_data(struct pwned_device *dev, DEV_PTR_T ptr);
int uninstall_all_data(struct pwned_device *dev);

int read_gadget(struct pwned_device *dev, DEV_PTR_T addr, void *dest, int len);
int write_gadget(struct pwned_device *dev, DEV_PTR_T addr, void *data, int len);
int execute_gadget(struct pwned_device *dev, DEV_PTR_T addr, unsigned long long *retval, int nargs, ...);
int memset_gadget(struct pwned_device *dev, DEV_PTR_T addr, unsigned char c, int len);
int memcpy_gadget(struct pwned_device *dev, DEV_PTR_T dest, DEV_PTR_T src, int len);

#endif //CHECKM8_TOOL_PAYLOAD_H
