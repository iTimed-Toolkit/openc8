#ifndef IPWNDFU_REWRITE_C_PAYLOAD_H
#define IPWNDFU_REWRITE_C_PAYLOAD_H

#include "checkm8.h"

#define PAYLOAD_SUCCESS     0
#define PAYLOAD_FAIL_DUP    -1

#define PAYLOAD_FOUND       0
#define PAYLOAD_NOT_FOUND   -1

typedef enum
{
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

int execute_payload(struct pwned_device *dev, PAYLOAD_T p, ...);

#endif //IPWNDFU_REWRITE_C_PAYLOAD_H
