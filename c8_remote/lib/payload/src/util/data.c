#include "bootrom_func.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE);

GLOBAL_CHAR(initialized, 0)
GLOBAL_PTR(installed_data, 0)
GLOBAL_INT(num_data, 0)

PAYLOAD_SECTION
void run()
{
    int i, j;
    struct data_args *args;
    struct cmd_resp *resp;
    struct install_args *base, *resp_install;

    if(GET_GLOBAL(initialized) == 0)
    {
        base = dev_malloc(NUM_ADDRESSES * sizeof(struct install_args));
        SET_GLOBAL(installed_data, base);
        SET_GLOBAL(initialized, 1);
    }
    else base = GET_GLOBAL(installed_data);

    args = GET_GLOBAL(addr_dfu_img_base);
    resp = GET_GLOBAL(addr_dfu_img_base);

    switch(args->cmd)
    {
        case DATA_STATUS:
            resp_install = GET_GLOBAL(addr_dfu_img_base) + sizeof(struct cmd_resp);
            j = 0;

            for(i = 0; i < GET_GLOBAL(num_data); i++)
            {
                if(base[i].addr != 0)
                {
                    resp_install[j].addr = base[i].addr;
                    resp_install[j].len = base[i].len;
                    j++;
                }
            }

            resp->status = CMD_SUCCESS;
            resp->args[0] = GET_GLOBAL(num_data);
            break;

        case DATA_INSTALL:
            for(i = 0; i < NUM_ADDRESSES; i++)
            {
                if(base[i].addr == 0)
                    break;
            }

            if(i == NUM_ADDRESSES)
            {
                resp->status = CMD_FAIL_FULL;
                return;
            }

            base[i].addr = (uint64_t) dev_malloc(args->len);
            base[i].len = args->len;
            dev_memcpy((void *) base[i].addr,
                       GET_GLOBAL(addr_dfu_img_base) + sizeof(struct data_args),
                       args->len);

            resp->status = CMD_SUCCESS;
            resp->args[0] = base[i].addr;
            resp->args[1] = i;

            SET_GLOBAL(num_data, GET_GLOBAL(num_data) + 1);

        case DATA_UNINSTALL:
            for(i = 0; i < NUM_ADDRESSES; i++)
            {
                if(base[i].addr == args->addr)
                    break;
            }

            if(i == NUM_ADDRESSES)
            {
                resp->status = CMD_FAIL_NOTFOUND;
                return;
            }

            dev_free((void *) base[i].addr);
            base[i].addr = 0;
            base[i].len = 0;

            resp->status = CMD_SUCCESS;
            resp->args[0] = (uint64_t) base[i].addr;
            resp->args[1] = i;

            SET_GLOBAL(num_data, GET_GLOBAL(num_data) - 1);

        default:
            resp->status = CMD_FAIL_INVALID;
            break;
    }
}

void _start()
{
    run();
}