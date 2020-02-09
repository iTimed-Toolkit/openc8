#include "util.h"
#include "brfunc_common.h"

TEXT_SECTION
void _start(unsigned long long next,
            unsigned long long arg0, unsigned long long arg1,
            unsigned long long arg2, unsigned long long arg3)
{
    int i;
    BOOTROM_FUNC clock_gate = ((BOOTROM_FUNC) 0x100009d4cull);
    for(i = 0x54 + 6; i > 6; i--)
    {
        if(i == 0x4b || i == 0x4a || i == 0x49 || i == 0x3e ||
           i == 0x1b || i == 0x17 || i == 0x11 || i == 0x10)
            continue;

        clock_gate(i, 0);
    }

    ((BOOTROM_FUNC) next)(arg0, arg1, arg2, arg3);
}