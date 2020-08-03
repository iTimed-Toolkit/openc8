#include "dev/addr.h"
#include "dev/shared_types.h"

#include "bootrom_func.h"
#include "arm64.h"

PAYLOAD_SECTION
static inline void patch_enter_soft_dfu()
{
    uint32_t *patch = (uint32_t *) 0x1800b2f40;
    *patch = 0x52801408;    // mov	w8, #0xa0
}

static inline void patch_enable_demote_boot()
{
    uint32_t *patch = (uint32_t *) 0x1800c29ec;
    *patch = 0x52800029;    // mov  w9, #0x1
}

static inline void patch_dfu_idProduct_string()
{
    int8_t *patch = (int8_t *) 0x18010d067;

    patch[0] = 'A';
    patch[1] = 'p';
    patch[2] = 'l';
    patch[3] = 'o';
    patch[4] = 'o';
}

PAYLOAD_SECTION
void patch_function()
{
    patch_enter_soft_dfu();
    patch_dfu_idProduct_string();
    patch_enable_demote_boot();
}

PAYLOAD_SECTION
void patch_trampoline()
{
    int i;
    const uint32_t arm64_ret = 0xd65f03c0, arm64_nop = 0xd503201f, align = 0x40;

    uint32_t *p_start, *p_end, p_len, t_offset;
    uint32_t *t_src = (uint32_t *) ADDR_TRAMPOLINE, *t_dst;

    // get bounds of the patch
    __asm__ volatile ("adr %0, patch_function" : "=r" (p_start));

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

void _start()
{
    // select regular mode for next boot
    *(uint32_t *) (WRITEABLE_ROM((void *) 0x100000664)) =
            0xd2800000; // mov x0, 0x0

    // enable demoted boot
    *(uint32_t *) (WRITEABLE_ROM((void *) 0x100006c54)) =
            0xd2800020; // mov x0, 0x1

    // hook into the last function before boot to patch iBoot
    uint64_t patch_addr;
    __asm__ volatile ("adr %0, patch_function" : "=r" (patch_addr));

    // we overwrite part of the previous function with the patcher address,
    // however this is okay for three reasons
    //  1. the previous function is only used when the bootrom is initialized
    //  2. the function we're stealing does nothing
    //  3. the function we're stealing is the last one called before loading iBoot
    *(uint64_t *) (WRITEABLE_ROM((void *) 0x10000b9b8)) = patch_addr;
    *(uint32_t *) (WRITEABLE_ROM((void *) 0x10000b9c0)) = 0x58ffffc0; // ldr x0, -8
    *(uint32_t *) (WRITEABLE_ROM((void *) 0x10000b9c4)) = 0xd61f0000; // br x0

    __asm__ volatile ("ic iallu\ndsb sy\nisb");

    *((int *) ADDR_DFU_RETVAL) = -1;
    *((char *) ADDR_DFU_STATUS) = 1;
    event_notify((struct event *) ADDR_DFU_EVENT);
}