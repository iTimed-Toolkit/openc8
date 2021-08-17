#include "dev_util.h"
#include "bootrom_func.h"
#include "util/armv8.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE)

PAYLOAD_SECTION
void run()
{
    struct hw_aes_args *args = GET_GLOBAL(addr_dfu_img_base);
    uint8_t msg[16], result[16];

    dev_memcpy(msg, args->msg, 16);

    // while(1) {
    gpio_write(0x1905, 1);
    gpio_write(0x1905, 0);

    hardware_aes((uint32_t) args->dir | args->mode,
                 msg, result, 16,
                 (uint32_t) args->type | args->size,
                 args->key,
                 0);

    gpio_write(0x1905, 1);
    gpio_write(0x1905, 0);

    dev_memcpy(msg, result, 16);
    dev_memcpy(GET_GLOBAL(addr_dfu_img_base), result, 16);

    task_pause(100);
    // }
}


void _start()
{
    run();
}