#include "bootrom_func.h"
#include "dev_cache.h"

PAYLOAD_SECTION
uint64_t run()
{
    int i;
    unsigned long long start, f;
    volatile unsigned long long val = 0;
    clean_inv_va((unsigned long long *) &val);

    start = get_ticks();
    for(i = 0; i < 10000000; i++)
    {
        val;
        clean_inv_va((unsigned long long *) &val);
    }

    return get_ticks() - start;
}

uint64_t _start()
{
    return run();
}