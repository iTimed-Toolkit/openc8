#include "util.h"
#include "brfunc_common.h"

PAYLOAD_SECTION
void task_sleep(unsigned int usec)
{
    ((BOOTROM_FUNC) ADDR_TASK_SLEEP)(usec);
}

TEXT_SECTION
unsigned long long _start(unsigned int usec)
{
    unsigned long long start, end;

    __asm__ volatile ("mrs %0, cntpct_el0" : "=r" (start));
    task_sleep(usec);
    __asm__ volatile ("mrs %0, cntpct_el0" : "=r" (end));

    return end - start;
}