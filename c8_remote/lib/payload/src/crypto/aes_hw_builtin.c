#include "dev_util.h"
#include "bootrom_func.h"
#include "util/armv8.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE)
//GLOBAL_PTR(addr_to_patch, 0x1000010ec);
//
//PAYLOAD_SECTION
//void patched_trigger()
//{
//
//}

PAYLOAD_SECTION
void run()
{
//    uint32_t old_val, new_val, *addr_to_patch = GET_GLOBAL(addr_to_patch);
    struct hw_aes_args *args = GET_GLOBAL(addr_dfu_img_base);

//    old_val = *WRITEABLE_ROM(addr_to_patch);
//    new_val = MOVZ(X20, 0, 0);
//    new_val = MOVK(X20, 32, 0);

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