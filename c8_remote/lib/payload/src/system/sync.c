#include "dev_util.h"

PAYLOAD_SECTION
void entry_sync()
{
    __asm__("dmb sy");
    __asm__("ic iallu");
    __asm__("dsb sy");
    __asm__("isb");
}

PAYLOAD_SECTION
void entry_async(){}