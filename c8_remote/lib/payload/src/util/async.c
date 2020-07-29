#include "bootrom_func.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE);

GLOBAL_CHAR(initialized, 0)
GLOBAL_PTR(installed_async, 0)
GLOBAL_INT(num_async, 0)

PAYLOAD_SECTION
void run()
{
    int i;
    void **base;
    struct async_args *args;
    struct cmd_resp *resp;

    if(GET_GLOBAL(initialized) == 0)
    {
        base = dev_malloc(NUM_ASYNC * sizeof(void *));
        SET_GLOBAL(installed_async, base);
        SET_GLOBAL(initialized, 1);
    }
    else base = GET_GLOBAL(installed_async);

    args = GET_GLOBAL(addr_dfu_img_base);
    resp = GET_GLOBAL(addr_dfu_img_base);

    switch(args->cmd)
    {
        case ASYNC_CREATE:
            for(i = 0; i < NUM_ASYNC; i++)
            {
                if(base[i] == 0)
                    break;
            }

            if(i == NUM_ASYNC)
            {
                resp->status = CMD_FAIL_FULL;
                return;
            }

            base[i] = task_new(args->name, (void *) args->func,
                               (void *) args->arg, 0x4000);

            resp->status = CMD_SUCCESS;
            resp->args[0] = i;
            break;

        case ASYNC_RUN:
            if(args->func >= NUM_ASYNC || base[args->func] == 0)
            {
                resp->status = CMD_FAIL_NOTFOUND;
                return;
            }

            task_run(base[args->func]);
            resp->status = CMD_SUCCESS;
            break;

        case ASYNC_FREE:
            if(args->func >= NUM_ASYNC || base[args->func] == 0)
            {
                resp->status = CMD_FAIL_NOTFOUND;
                return;
            }

            task_free(base[args->func]);
            base[args->func] = 0;
            resp->status = CMD_SUCCESS;
            break;

        default:
            resp->status = CMD_FAIL_INVALID;
            break;
    }
}

void _start()
{
    run();
}