#include "dev_util.h"
#include "dev/addr.h"

PAYLOAD_SECTION
void load_sync_entry()
{
    uint64_t addr = ADDR_SYNC_ENTRY;
    __asm__ volatile("mov x9, %0"           :: "i" (addr & 0xFFFFu));
    __asm__ volatile("movk x9, %0, LSL #16" :: "i" ((addr & 0xFFFF0000u) >> 16u));
    __asm__ volatile("movk x9, %0, LSL #32" :: "i" ((addr & 0xFFFF00000000u) >> 32u));
    __asm__ volatile("movk x9, %0, LSL #48" :: "i" ((addr & 0xFFFF000000000000u) >> 48u));

}