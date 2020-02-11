#include "dev_util.h"

extern uint64_t entry_sync(uint64_t *args);
extern uint64_t entry_async(uint64_t *base);

TEXT_SECTION
uint64_t _start(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t arg7)
{
    uint64_t entry, args[8];
    __asm__ volatile ("mov %0, x30" : "=r" (entry));

    if(entry == 0xbea /* todo: correct entry */)
    {
        args[0] = arg0;
        args[1] = arg1;
        args[2] = arg2;
        args[3] = arg3;
        args[4] = arg4;
        args[5] = arg5;
        args[6] = arg6;
        args[7] = arg7;

        return entry_sync(args);
    }
    else
        return entry_async((uint64_t *) arg0);
}