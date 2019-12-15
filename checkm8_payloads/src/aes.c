#include "brfunc_aes.h"
#include "brfunc_timing.h"
#include "brfunc_sep.h"

#include "util.h"

PAYLOAD_SECTION
int aes_hw_crypto_command(unsigned int cmd,
                          void *src,
                          void *dst,
                          int len,
                          unsigned int opts,
                          void *key,
                          void *iv)
{
    int seeded;
    long start = 0, timeout = 0;
    CLOCK_GATE(0x3C, 1);

    seeded = DPA_SEEDED();
    if(!seeded)
    {
        SEP_CREATE_SEND_DPA_MESSAGE();
        start = SYSTEM_TIME();

        while(!seeded && !timeout)
        {
            seeded = DPA_SEEDED();
            timeout = TIME_HAS_ELAPSED(start, 1000);
        }
    }

    if(timeout) return -1;

    unsigned int key_command = CREATE_KEY_COMMAND(0, 0, 0, 0, 1, 0, 0, 0);
    *rAES_INT_STATUS = 0x20;
    *rAES_CONTROL = 1;

    PUSH_COMMAND_KEY(key_command, key);
    PUSH_COMMAND_IV(0, 0, 0, iv);
    PUSH_COMMAND_DATA(0, 0, src, dst, len);
    PUSH_COMMAND_FLAG(0, 1, 1);
    WAIT_FOR_COMMAND_FLAG();

    *rAES_CONTROL = 2;
    CLOCK_GATE(0x3C, 0);
    return 0;
}

TEXT_SECTION
int _start(unsigned int cmd,
           void *src,
           void *dst,
           int len,
           unsigned int opts,
           void *key,
           void *iv)
{
    return aes_hw_crypto_command(cmd, src, dst, len, opts, key, iv);
}