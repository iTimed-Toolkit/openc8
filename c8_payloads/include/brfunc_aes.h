#ifndef CHECKM8_TOOL_BRFUNC_AES_H
#define CHECKM8_TOOL_BRFUNC_AES_H

#include "brfunc_common.h"

PAYLOAD_SECTION
int aes_hw_crypto_cmd(unsigned long long cmd,
                      unsigned char *src, unsigned char *dst,
                      int len, unsigned long long opts,
                      unsigned char *key, unsigned char *iv)
{
    return ((BOOTROM_FUNC) ADDR_AES_HW_CRYPTO_CMD)(cmd, src, dst, len, opts, key, iv);
}

#define CREATE_KEY_COMMAND          ((BOOTROM_FUNC) ADDR_CREATE_KEY_COMMAND)
#define PUSH_COMMAND_KEY            ((BOOTROM_FUNC) ADDR_PUSH_COMMAND_KEY)
#define PUSH_COMMAND_IV             ((BOOTROM_FUNC) ADDR_PUSH_COMMAND_IV)
#define PUSH_COMMAND_DATA           ((BOOTROM_FUNC) ADDR_PUSH_COMMAND_DATA)
#define PUSH_COMMAND_FLAG           ((BOOTROM_FUNC) ADDR_PUSH_COMMAND_FLAG)
#define WAIT_FOR_COMMAND_FLAG       ((BOOTROM_FUNC) ADDR_WAIT_FOR_COMMAND)

#define rAES_INT_STATUS             (long *) ADDR_AES_CONTROL
#define rAES_CONTROL                (long *) ADDR_AES_STATUS

#endif //CHECKM8_TOOL_BRFUNC_AES_H
