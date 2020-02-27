#include "dev/addr.h"
#include "bootrom_func.h"

PAYLOAD_SECTION
void fix_heap()
{
    *((unsigned long long *) 0x1801b91a0) = 0x80 / 0x40;
    *((unsigned long long *) 0x1801b91a8) = ((0x840u / 0x40) << 2u);
    *((unsigned long long *) 0x1801b91b0) = 0x80;
    *((unsigned long long *) 0x1801b91b8) = 0;

    *((unsigned long long *) 0x1801b9220) = 0x80 / 0x40;
    *((unsigned long long *) 0x1801b9228) = ((0x80u / 0x40) << 2u);
    *((unsigned long long *) 0x1801b9230) = 0x80;
    *((unsigned long long *) 0x1801b9238) = 0;

    *((unsigned long long *) 0x1801b92a0) = 0x80 / 0x40;
    *((unsigned long long *) 0x1801b92a8) = ((0x80u / 0x40) << 2u);
    *((unsigned long long *) 0x1801b92b0) = 0x80;
    *((unsigned long long *) 0x1801b92b8) = 0;

    __asm__ volatile ("dmb sy");
    calc_chksum((unsigned long long *) 0x1801b9180,
                (unsigned long long *) 0x1801b91a0,
                32,
                (unsigned long long *) 0x180080640);

    calc_chksum((unsigned long long *) 0x1801b9200,
                (unsigned long long *) 0x1801b9220,
                32,
                (unsigned long long *) 0x180080640);

    calc_chksum((unsigned long long *) 0x1801b9280,
                (unsigned long long *) 0x1801b92a0,
                32,
                (unsigned long long *) 0x180080640);

    __asm__ volatile ("dmb sy");
    check_all_chksums();
}

void entry_sync(unsigned long long *self)
{
    fix_heap();

    *(ADDR_DFU_RETVAL) = -1;
    *(ADDR_DFU_STATUS) = 1;

    event_notify((struct event *) ADDR_DFU_EVENT);
    dev_free(self);
}

void entry_async(uint64_t *base){}