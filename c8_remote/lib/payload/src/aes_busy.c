#include "bootrom_func.h"

PAYLOAD_SECTION
uint64_t entry_sync(uint64_t *args)
{
    int i, j;
    unsigned char src_data[16];

    unsigned char *src = (unsigned char *) args[0];
    unsigned char *dst = (unsigned char *) args[1];
    unsigned char *key = (unsigned char *) args[2];
    int rep = (int) args[3];

    for(j = 0; j < 16; j++)
    {
        src_data[j] = src[j];
    }

    for(i = 0; i < rep; i++)
    {
        if(i % 2 == 0) hardware_aes(16, src_data, dst, 16, 0, key, 0);
        else hardware_aes(16, dst, src_data, 16, 0, key, 0);
    }

    return 0;
}

PAYLOAD_SECTION
uint64_t entry_async(uint64_t *base)
{
    return 0;
}