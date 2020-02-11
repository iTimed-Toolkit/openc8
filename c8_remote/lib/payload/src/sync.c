#include "dev_util.h"

PAYLOAD_SECTION
extern uint64_t entry_sync(uint64_t *args)
{
    __asm__("dmb sy");
    __asm__("ic iallu");
    __asm__("dsb sy");
    __asm__("isb");

    return 0;
}

PAYLOAD_SECTION
extern uint64_t entry_async(uint64_t *base)
{
    return 0;
}