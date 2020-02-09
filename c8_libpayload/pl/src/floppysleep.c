#include "brfunc_common.h"
#include "util.h"

extern unsigned long long fs_routine(void);

extern unsigned long long fs_load(float *dividend, int divisor_base);
// extern unsigned long long check_subnormal();

PAYLOAD_SECTION
unsigned int is_subnormal(float val)
{
    unsigned int bytes = *((unsigned int *) &val);
    bytes = bytes >> 23u;

    if(bytes & 0x7u)
    {
        return 0;
    }
    else return 1;
}

TEXT_SECTION
unsigned long long _start(float *init_a)
{
    int i;
    volatile int j = 0;

    unsigned long long start, end, report;
    unsigned long long timer_deadline_enter = 0x10000b874;
    unsigned long long halt = 0x1000004fc;

    while(1)
    {
        __asm__ volatile ("isb\n\rmrs %0, cntpct_el0" : "=r" (start));
        fs_load(init_a, 1);
        for(i = 0; i < 8; i++) fs_routine();
        __asm__ volatile ("isb\n\rmrs %0, cntpct_el0" : "=r" (end));

        if(2 * end - start - 64 > 0)
        {
            ((BOOTROM_FUNC) timer_deadline_enter)(2 * end - start - 64, ((BOOTROM_FUNC) 0x10000b924));
            ((BOOTROM_FUNC) halt)();
        }

        __asm__ volatile ("isb\n\rmrs %0, cntpct_el0" : "=r" (report));
        j++;
    }

    return end - start;
}