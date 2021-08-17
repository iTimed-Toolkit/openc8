#include "dev/addr.h"
#include "dev/shared_types.h"
#include "util/armv8.h"

#include "bootrom_func.h"

static inline void patch_enable_demote_boot()
{
    uint32_t *patch = (uint32_t *) 0x1800c3334;
    *patch = MOVZ(X9, 0x1, 0);    // mov  w9, #0x1
}

static inline void patch_avoid_start_sep()
{
    uint32_t *patch = (uint32_t *) 0x1800b3be4;
    *patch = MOVZ(X0, 0, 0);
}

PAYLOAD_SECTION
void patch_function()
{
//    patch_enter_soft_dfu();
//    patch_remove_dfu_img_load();
//    patch_iserial();
//    patch_boot_after_abort();
//    patch_jump_to_pongo();

    patch_enable_demote_boot();
    patch_avoid_start_sep();

    __asm__ volatile ("b 0");
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