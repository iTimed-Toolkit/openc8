#include "dev/addr.h"
#include "bootrom_func.h"

PAYLOAD_SECTION
void entry_sync(uint64_t addr_hook)
{
    uint64_t *bs_task_sp = ((uint64_t *) ADDR_BOOTSTRAP_TASK + (0x128 / 8));
    uint64_t *bs_task_stack = (uint64_t *) *bs_task_sp;

    while(1)
    {
        if(*bs_task_stack == ADDR_GETDFU_EXIT)
        {
            *bs_task_stack = addr_hook;
            break;
        }

        bs_task_stack++;
    }

    *(ADDR_DFU_RETVAL) = -1;
    *(ADDR_DFU_STATUS) = 1;

    event_notify((struct event *) ADDR_DFU_EVENT);
}

PAYLOAD_SECTION
void entry_async(){}