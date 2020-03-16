#include "dev/addr.h"
#include "dev/types.h"

#include "bootrom_func.h"
#include "dev_cache.h"

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
void trampoline_function()
{
    int8_t *base = (int8_t *) 0x18010a36f;
    uint64_t addr, arg;

    __asm__ volatile ("str x0, %0" : "=m" (addr));
    __asm__ volatile ("str x1, %0" : "=m" (arg));

    base[9] = ' ';
    base[10] = 'H';
    base[11] = 'E';
    base[12] = 'L';
    base[13] = 'L';
    base[14] = 'O';
    base[15] = ' ';
    base[16] = 'W';
    base[17] = 'O';
    base[18] = 'R';
    base[19] = 'L';
    base[20] = 'D';
    base[21] = ' ';

    __asm__ volatile ("ldr x0, %0"::"m" (addr));
    __asm__ volatile ("ldr x1, %0"::"m" (arg));
}

PAYLOAD_SECTION
void patch_trampoline()
{
    int i;
    const uint32_t arm64_ret = 0xd65f03c0, arm64_nop = 0xd503201f, align = 0x40;

    uint32_t *p_start, *p_end, p_len, t_offset;
    uint32_t *t_src = (uint32_t *) ADDR_TRAMPOLINE, *t_dst;

    // get bounds of the patch
    __asm__ volatile ("adr %0, %1" : "=r" (p_start) : "X" (&trampoline_function));

    p_end = p_start;
    while(*p_end != arm64_ret) p_end++;

    p_len = p_end - p_start;
    t_offset = ((p_len / align) + 1) * align;

    // copy the trampoline to new location
    t_dst = t_src + t_offset;
    for(i = LEN_TRAMPOLINE - 1; i >= 0; i--)
        t_dst[i] = t_src[i];

    // copy patch to beginning
    for(i = 0; i < p_len; i++)
        t_src[i] = p_start[i];

    // nop slide between patch and trampoline
    for(i = p_len; i < t_offset; i++)
        t_src[i] = arm64_nop;
}

PAYLOAD_SECTION
void entry_sync()
{
    uint64_t *bootstrap_sp = (uint64_t *) ADDR_BOOTSTRAP_TASK + 0x25;
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

    fix_heap();
    patch_trampoline();

    *(ADDR_DFU_RETVAL) = -1;
    *(ADDR_DFU_STATUS) = 1;
    event_notify(ADDR_DFU_EVENT);
}

PAYLOAD_SECTION
void entry_async()
{}