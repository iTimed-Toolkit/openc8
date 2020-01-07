#include "util.h"
#include "brfunc_aes.h"


TEXT_SECTION
int _start(void *src, void *dst, void *key, int rep)
{
    int i, j;
    unsigned char src_data[16];
    for(j = 0; j < 16; j++)
    {
        src_data[j] = ((unsigned char *) src)[j];
    }

    for(i = 0; i < rep; i++)
    {
        if(i % 2 == 0) aes_hw_crypto_cmd(16, src_data, dst, 16, 0, key, 0);
        else aes_hw_crypto_cmd(16, dst, src_data, 16, 0, key, 0);
    }

    return 0;
}