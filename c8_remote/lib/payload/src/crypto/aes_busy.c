#include "bootrom_func.h"

PAYLOAD_SECTION
void entry_sync(uint8_t *src, uint8_t *dst, uint8_t *key, int32_t rep)
{
    int i, j;
    unsigned char src_data[16];

    for(j = 0; j < 16; j++)
    {
        src_data[j] = src[j];
    }

    for(i = 0; i < rep; i++)
    {
        if(i % 2 == 0) hardware_aes(16, src_data, dst, 16, 0, key, 0);
        else hardware_aes(16, dst, src_data, 16, 0, key, 0);
    }
}

PAYLOAD_SECTION
void entry_async(uint64_t *base){}