#ifndef CHECKM8_TOOL_PAYLOAD_H
#define CHECKM8_TOOL_PAYLOAD_H

#include "checkm8.h"

typedef enum
{
    PAYLOAD_AES_BUSY,
    PAYLOAD_AES_SW_BERN,
    PAYLOAD_AES_SW_CORR,
    PAYLOAD_CACHELIB,
    PAYLOAD_EXIT_USB_TASK,
    PAYLOAD_FLOPPYSLEEP,
    PAYLOAD_SYNC,
} PAYLOAD_T;

typedef enum
{
    SRAM,
    DRAM
} LOCATION_T;

#define DEV_PTR_NULL       -1ull
typedef unsigned long long DEV_PTR_T;

int install_payload(struct pwned_device *dev, PAYLOAD_T p, LOCATION_T loc);
int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p);
int uninstall_all_payloads(struct pwned_device *dev);
DEV_PTR_T get_payload_address(struct pwned_device *dev, PAYLOAD_T p);

struct dev_cmd_resp *execute_payload(struct pwned_device *dev, PAYLOAD_T p, int response_len, int nargs, ...);
DEV_PTR_T setup_payload_async(struct pwned_device *dev, PAYLOAD_T p, int bufsize, int nargs, ...);
int run_payload_async(struct pwned_device *dev, PAYLOAD_T p);
int kill_payload_async(struct pwned_device *dev, PAYLOAD_T p, DEV_PTR_T buf_addr);

DEV_PTR_T install_data(struct pwned_device *dev, LOCATION_T loc, unsigned char *data, int len);
int uninstall_data(struct pwned_device *dev, DEV_PTR_T ptr);
int uninstall_all_data(struct pwned_device *dev);

struct dev_cmd_resp *read_gadget(struct pwned_device *dev, DEV_PTR_T addr, int len);
struct dev_cmd_resp *write_gadget(struct pwned_device *dev, DEV_PTR_T addr, unsigned char *data, int len);
struct dev_cmd_resp *execute_gadget(struct pwned_device *dev, DEV_PTR_T addr, int response_len, int nargs, ...);

#endif //CHECKM8_TOOL_PAYLOAD_H
