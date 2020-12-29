#include "dev_util.h"
#include "bootrom_func.h"
#include "util/armv8.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE)

PAYLOAD_SECTION
void run()
{
    struct hw_aes_args *args = GET_GLOBAL(addr_dfu_img_base);

    gpio_write(0x1905, 1);
    gpio_write(0x1905, 0);

    hardware_aes((uint32_t) args->dir | args->mode,
                 args->msg,
                 GET_GLOBAL(addr_dfu_img_base),
                 16,
                 (uint32_t) args->type | args->size,
                 args->key,
                 0);

    gpio_write(0x1905, 1);
    gpio_write(0x1905, 0);
}


void _start()
{
    run();
}