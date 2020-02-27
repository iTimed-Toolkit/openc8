#ifndef CHECKM8_TOOL_DEV_CACHE_H
#define CHECKM8_TOOL_DEV_CACHE_H

#include "dev_util.h"

PAYLOAD_SECTION
static inline unsigned long long get_ccsidr_el1()
{
    unsigned long long cacheconfig = 0;
    __asm__ volatile ("mrs %0, ccsidr_el1" : "=r" (cacheconfig));
    return cacheconfig;
}

PAYLOAD_SECTION
static inline void sel_ccsidr_el1(unsigned int level, unsigned int i_or_d)
{
    unsigned long long cachesel = (level & 0b111u) << 1u | (i_or_d & 0b1u);
    __asm__ volatile ("msr csselr_el1, %0"::"r" (cachesel));
}

PAYLOAD_SECTION
static inline unsigned long long get_ctr_el0()
{
    unsigned long long cacheconfig;
    __asm__ volatile ("mrs %0, CTR_EL0" : "=r" (cacheconfig));
    return cacheconfig;
}

PAYLOAD_SECTION
static inline void inv_l1_setway(unsigned int set, unsigned int way)
{
    unsigned long long val = ((way & 0b11u) << 30u) | ((set & 0xFFu) << 6u);
    __asm__ volatile ("dc isw, %0"::"r" (val));
}

PAYLOAD_SECTION
static inline void clean_l1_setway(unsigned int set, unsigned int way)
{
    unsigned long long val = ((way & 0b11u) << 30u) | ((set & 0xFFu) << 6u);
    __asm__ volatile ("dc csw, %0"::"r" (val));
}

PAYLOAD_SECTION
static inline void clean_inv_l1_setway(unsigned int set, unsigned int way)
{
    unsigned long long val = ((way & 0b11u) << 30u) | ((set & 0xFFu) << 6u);
    __asm__ volatile ("dc cisw, %0"::"r" (val));
}

PAYLOAD_SECTION
static inline void inv_va(void *addr)
{
    __asm__ volatile ("dc ivac, %0"::"r" (addr));
}

PAYLOAD_SECTION
static inline void clean_inv_va(void *addr)
{
    __asm__ volatile ("dc ivac, %0"::"r" (addr));
}

#endif //CHECKM8_TOOL_DEV_CACHE_H
