#ifndef CHECKM8_TOOL_PAYLOAD_H
#define CHECKM8_TOOL_PAYLOAD_H

#include "checkm8.h"
#include "libpayload.h"
#include "dev/types.h"

typedef enum
{
    SRAM,
    DRAM
} LOCATION_T;

int install_payload(struct pwned_device *dev, PAYLOAD_T p, LOCATION_T loc);
int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p);
int uninstall_all_payloads(struct pwned_device *dev);
DEV_PTR_T get_payload_address(struct pwned_device *dev, PAYLOAD_T p);

struct dev_cmd_resp *execute_payload(struct pwned_device *dev, PAYLOAD_T p, int response_len, int nargs, ...);
int async_payload_create(struct pwned_device *dev, PAYLOAD_T p, DEV_PTR_T arg);
int async_payload_run(struct pwned_device *dev, PAYLOAD_T p);
int async_payload_kill(struct pwned_device *dev, PAYLOAD_T p, DEV_PTR_T buf_addr);

DEV_PTR_T get_address(struct pwned_device *dev, LOCATION_T l, int len);
int free_address(struct pwned_device *dev, LOCATION_T l, DEV_PTR_T ptr);

DEV_PTR_T install_data(struct pwned_device *dev, LOCATION_T loc, void *data, int len);
DEV_PTR_T install_data_offset(struct pwned_device *dev, LOCATION_T loc, void *data, int len, unsigned int offset);

int uninstall_data(struct pwned_device *dev, DEV_PTR_T ptr);
int uninstall_all_data(struct pwned_device *dev);

struct dev_cmd_resp *read_gadget(struct pwned_device *dev, DEV_PTR_T addr, int len);
struct dev_cmd_resp *write_gadget(struct pwned_device *dev, DEV_PTR_T addr, void *data, int len);
struct dev_cmd_resp *execute_gadget(struct pwned_device *dev, DEV_PTR_T addr, int response_len, int nargs, ...);
struct dev_cmd_resp *memset_gadget(struct pwned_device *dev, DEV_PTR_T addr, unsigned char c, int len);
struct dev_cmd_resp *memcpy_gadget(struct pwned_device *dev, DEV_PTR_T dest, DEV_PTR_T src, int len);

#endif //CHECKM8_TOOL_PAYLOAD_H
