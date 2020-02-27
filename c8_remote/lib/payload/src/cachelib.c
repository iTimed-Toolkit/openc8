#include "bootrom_func.h"
#include "dev_cache.h"


PAYLOAD_SECTION
unsigned long long l1_experiment()
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

PAYLOAD_SECTION
unsigned long long entry_sync()
{
    return l1_experiment();
}

PAYLOAD_SECTION
void entry_async()
{

}