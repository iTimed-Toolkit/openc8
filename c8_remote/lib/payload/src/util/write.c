#include "bootrom_func.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE)

PAYLOAD_SECTION
void run()
{
    struct rw_args *args = GET_GLOBAL(addr_dfu_img_base);
    struct cmd_resp *resp = GET_GLOBAL(addr_dfu_img_base);

    dev_memcpy((void *) args->addr,
               GET_GLOBAL(addr_dfu_img_base) + sizeof(struct rw_args),
               args->len);

    resp->status = CMD_SUCCESS;
    resp->args[0] = 0;
}

void _start()
{
    run();
}