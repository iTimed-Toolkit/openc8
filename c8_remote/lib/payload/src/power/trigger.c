#include "dev_util.h"
#include "bootrom_func.h"

GLOBAL_PTR(rom_rx_ptr, 0x1800B0400)
GLOBAL_LONG(rom_rx, 0x1000006a5)

GLOBAL_PTR(sram_rw_ptr, 0x1800B0600)
GLOBAL_LONG(sram_rw, 0x60000180000625)

GLOBAL_PTR(sram_rx_ptr, 0x1800B0608)
GLOBAL_LONG(sram_rx, 0x1800006a5)

extern void run();

PAYLOAD_SECTION
void try_patch_pt()
{
    *(long long *) GET_GLOBAL(rom_rx_ptr) = GET_GLOBAL(rom_rx);
    *(long long *) GET_GLOBAL(sram_rw_ptr) = GET_GLOBAL(sram_rw);
    *(long long *) GET_GLOBAL(sram_rx_ptr) = GET_GLOBAL(sram_rx);

    write_ttbr0(ADDR_DFU_IMG_BASE);
    tlbi();

    map_range(0x102000000, 0x100000000,0x2000000, normal_rw);

    write_ttbr0(0x1800A0000);
    tlbi();
}

void _start()
{
    try_patch_pt();
}
              