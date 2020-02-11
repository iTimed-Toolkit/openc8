#include "brfunc_common.h"
#include "util.h"

PAYLOAD_SECTION
void fix_heap()
{
    BOOTROM_FUNC calculate_checksum = ((BOOTROM_FUNC) 0x10000ee20);
    BOOTROM_FUNC heap_verify = ((BOOTROM_FUNC) 0x10000f8b4);

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
    calculate_checksum((unsigned long long *) 0x1801b9180,
                       (unsigned long long *) 0x1801b91a0,
                       32,
                       (unsigned long long *) 0x180080640);

    calculate_checksum((unsigned long long *) 0x1801b9200,
                       (unsigned long long *) 0x1801b9220,
                       32,
                       (unsigned long long *) 0x180080640);

    calculate_checksum((unsigned long long *) 0x1801b9280,
                       (unsigned long long *) 0x1801b92a0,
                       32,
                       (unsigned long long *) 0x180080640);

    __asm__ volatile ("dmb sy");
    heap_verify();
}

TEXT_SECTION
void _start(unsigned long long ptr_self)
{
    unsigned int *completion = (unsigned int *) 0x180088ac8;
    unsigned char *dfu_done = (unsigned char *) 0x180088ac0;

    unsigned long long *dfu_event = (unsigned long long *) 0x180088af0;
    BOOTROM_FUNC event_signal = ((BOOTROM_FUNC) 0x10000aee8);
    BOOTROM_FUNC free = ((BOOTROM_FUNC) 0x10000f1b0);

    fix_heap();

    *completion = -1;
    *dfu_done = 1;

    event_signal(dfu_event);
    free(ptr_self);
}