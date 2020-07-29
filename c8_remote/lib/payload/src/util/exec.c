#include "bootrom_func.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE)

PAYLOAD_SECTION
void run()
{
    struct exec_args *args = GET_GLOBAL(addr_dfu_img_base);
    struct cmd_resp *resp = GET_GLOBAL(addr_dfu_img_base);

    uint64_t arg0 = args->args[0],
            arg1 = args->args[1],
            arg2 = args->args[2],
            arg3 = args->args[3],
            arg4 = args->args[4],
            arg5 = args->args[5],
            arg6 = args->args[6],
            arg7 = args->args[7],
            retval;

    retval = args->addr(arg0, arg1, arg2, arg3,
                        arg4, arg5, arg6, arg7);

    resp->status = CMD_SUCCESS;
    resp->args[0] = retval;
}

void _start()
{
    run();
}