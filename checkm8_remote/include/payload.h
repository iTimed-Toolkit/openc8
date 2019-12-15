#ifndef IPWNDFU_REWRITE_C_PAYLOAD_H
#define IPWNDFU_REWRITE_C_PAYLOAD_H

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

int install_payload(struct pwned_device *dev, PAYLOAD_T p, LOCATION_T loc);
int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p);

int execute_payload(struct pwned_device *dev, PAYLOAD_T p, int nargs, ...);

#endif //IPWNDFU_REWRITE_C_PAYLOAD_H
