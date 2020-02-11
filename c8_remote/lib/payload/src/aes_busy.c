#include "bootrom_func.h"

TEXT_SECTION
int _start(void *src, void *dst, void *key, int rep)
{
    int i, j;
    unsigned char src_data[16];
    for(j = 0; j < 16; j++)
    {
        src_data[j] = ((unsigned char *) src)[j];
    }

//    task_sleep(100);
    for(i = 0; i < rep; i++)
    {
        if(i % 2 == 0) hardware_aes(16, src_data, dst, 16, 0, key, 0);
        else hardware_aes(16, dst, src_data, 16, 0, key, 0);
        //       task_sleep(15);
    }

    return 0;
}