#ifndef CHECKM8_TOOL_PAYLOAD_H
#define CHECKM8_TOOL_PAYLOAD_H

#include "checkm8.h"

#define PAYLOAD_AES_BIN     CHECKM8_BIN_BASE "payloads/payload_aes.bin"
#define PAYLOAD_SYSREG_BIN  CHECKM8_BIN_BASE "payloads/payload_sysreg.bin"
#define PAYLOAD_SYNC_BIN    CHECKM8_BIN_BASE "payloads/payload_sync.bin"

typedef enum
{
    PAYLOAD_SYNC,
    PAYLOAD_AES,
    PAYLOAD_SYSREG
} PAYLOAD_T;

typedef enum
{
    SRAM,
    DRAM
} LOCATION_T;

#define RESP_VALUE(buf, type, i) ((type *) buf)[i]

int install_payload(struct pwned_device *dev, PAYLOAD_T p, LOCATION_T loc);
int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p);
struct dev_cmd_resp *execute_payload(struct pwned_device *dev, PAYLOAD_T p, int response_len, int nargs, ...);

struct dev_cmd_resp *read_gadget(struct pwned_device *dev, long long addr, int len);
struct dev_cmd_resp *write_gadget(struct pwned_device *dev, long long addr, unsigned char *data, int len);
struct dev_cmd_resp *execute_gadget(struct pwned_device *dev, long long addr, int response_len, int nargs, ...);

#endif //CHECKM8_TOOL_PAYLOAD_H
