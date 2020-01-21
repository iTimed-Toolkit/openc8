#include "util.h"

TEXT_SECTION
unsigned long long _start()
{
//    unsigned long long platform_quiesce_hardware = 0x100007dd0;
//    unsigned long long enter_critical_section = 0x10000a4b8;
//    unsigned long long halt = 0x1000004fc;
//    unsigned long long timer_deadline_enter = 0x10000b874;
//    unsigned long long now, later;
//
//    ((BOOTROM_FUNC) platform_quiesce_hardware)();
//    //((BOOTROM_FUNC) enter_critical_section)();
//
//    __asm__ volatile ("mrs %0, cntpct_el0" : "=r" (now));
//    ((BOOTROM_FUNC) timer_deadline_enter)(now + (24000000) - 64, ((BOOTROM_FUNC) 0x10000b924));
//    ((BOOTROM_FUNC) halt)();
//    __asm__ volatile ("mrs %0, cntpct_el0" : "=r" (later));

    volatile unsigned long long regval = 0xffff;
    __asm__ volatile ("mrs %0, fpcr" : "=r" (regval));
    regval = (1u << 24u);
    __asm__ volatile ("msr fpcr, %0" : "=r" (regval));

    return regval;
}