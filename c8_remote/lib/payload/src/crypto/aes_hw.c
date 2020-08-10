#include "bootrom_func.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE)

PAYLOAD_SECTION
void run()
{
    struct hw_aes_args *args = GET_GLOBAL(addr_dfu_img_base);
    hardware_aes((uint32_t) args->dir | args->mode,
                 args->msg,
                 GET_GLOBAL(addr_dfu_img_base),
                 16,
                 (uint32_t) args->type | args->size,
                 args->key,
                 0);
}


void _start()
{
    run();
}