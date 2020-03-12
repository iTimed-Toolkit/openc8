#include "bootrom_func.h"

extern uint64_t fs_routine(void);
extern uint64_t fs_load(float *dividend, int divisor_base);
// extern uint64_t check_subnormal();

//PAYLOAD_SECTION
//unsigned int is_subnormal(float val)
//{
//    unsigned int bytes = *((unsigned int *) &val);
//    bytes = bytes >> 23u;
//
//    if(bytes & 0x7u)
//    {
//        return 0;
//    }
//    else return 1;
//}

PAYLOAD_SECTION
uint64_t floppysleep_iteration(float *init)
{
    int i;
    uint64_t start, end, report;

    __asm__ volatile ("isb\n\rmrs %0, cntpct_el0" : "=r" (start));
    fs_load(init, 1);
    for(i = 0; i < 128; i++) fs_routine();
    __asm__ volatile ("isb\n\rmrs %0, cntpct_el0" : "=r" (end));

    if(2 * end - start - 64 > 0)
    {
        timer_register_int(2 * end - start - 64);
        wfi();
    }

    __asm__ volatile ("isb\n\rmrs %0, cntpct_el0" : "=r" (report));
    return end - start;
}

PAYLOAD_SECTION
uint64_t entry_sync(float *init_ptr)
{
    return floppysleep_iteration(init_ptr);
}

PAYLOAD_SECTION
void entry_async(uint64_t *args)
{
    float *init_ptr = (float *) args[0];
    args[0] = 0;

    while(1)
    {
        floppysleep_iteration(init_ptr);

        if(args[0] % 1000000 == 0) task_resched();
        args[0]++;
    }
}
