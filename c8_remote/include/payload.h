#ifndef CHECKM8_TOOL_PAYLOAD_H
#define CHECKM8_TOOL_PAYLOAD_H

#include "checkm8.h"

typedef enum
{
    PAYLOAD_AES,
    PAYLOAD_AES_BUSY,
    PAYLOAD_AES_SW,
    PAYLOAD_BOOTSTRAP,
    PAYLOAD_EXIT_USB_TASK,
    PAYLOAD_FLOPPYSLEEP,
    PAYLOAD_SYNC,
    PAYLOAD_SYSREG,
    PAYLOAD_TASK_SLEEP_TEST
} PAYLOAD_T;

typedef enum
{
    SRAM,
    DRAM
} LOCATION_T;

int install_payload(struct pwned_device *dev, PAYLOAD_T p, LOCATION_T loc);
int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p);
struct dev_cmd_resp *execute_payload(struct pwned_device *dev, PAYLOAD_T p, int response_len, int nargs, ...);

struct dev_cmd_resp *read_gadget(struct pwned_device *dev, long long addr, int len);
struct dev_cmd_resp *write_gadget(struct pwned_device *dev, long long addr, unsigned char *data, int len);
struct dev_cmd_resp *execute_gadget(struct pwned_device *dev, long long addr, int response_len, int nargs, ...);

#endif //CHECKM8_TOOL_PAYLOAD_H
