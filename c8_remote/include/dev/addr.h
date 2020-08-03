#ifndef CHECKM8_TOOL_ADDR_H
#define CHECKM8_TOOL_ADDR_H

#include "checkm8_config.h"
#include "shared_types.h"

#if CHECKM8_PLATFORM == 8010

/* Crypto */
#define ADDR_HARDWARE_AES                   0x100000f0c
#define ADDR_GET_RANDOM                     0x1000113e0
#define ADDR_GET_ENTROPY                    0x1000013d4
#define ADDR_SHA1                           0x10000cc90

/* Timing */
#define ADDR_CLOCK_GATE                     0x100009d4c
#define ADDR_GET_TIME                       0x10000b0e0
#define ADDR_GET_TICKS                      0x10000041c
#define ADDR_TIMER_REGISTER_INT             0x10000b874
#define ADDR_WFI                            0x1000004fc

/* Tasking */
#define ADDR_TASK_NEW                       0x10000a9ac
#define ADDR_TASK_RUN                       0x10000ac18
#define ADDR_TASK_PAUSE                     0x10000adf0
#define ADDR_TASK_RESCHED                   0x10000aaa8
#define ADDR_TASK_FREE                      0x10000aa20
#define ADDR_TASK_EXIT                      0x10000ac5c

#define ADDR_EVENT_NEW                      0x10000aed4
#define ADDR_EVENT_NOTIFY                   0x10000aee8
#define ADDR_EVENT_WAIT                     0x10000af3c
#define ADDR_EVENT_TRY                      0x10000af7c

#define ADDR_ENTER_CRITICAL                 0x10000a4b8
#define ADDR_EXIT_CRITICAL                  0x10000a514

#define ADDR_BOOTSTRAP_TASK                 0x180080200

/* Heap */
#define ADDR_CALC_CHKSUM                    0x10000ee20
#define ADDR_CHECK_BLOCK_CKSUM              0x10000f138
#define ADDR_CHECK_ALL_CHKSUMS              0x10000f8b4

#define ADDR_DEV_MALLOC                     0x10000efe0
#define ADDR_DEV_MEMALIGN                   0x10000f380
#define ADDR_DEV_FREE                       0x10000f1b0

#define ADDR_HEAP_COOKIE                    0x180080640
#define ADDR_HEAP_BASE                      0x1801b4000
#define ADDR_HEAP_END                       0x1801fffc0

/* Memory */
#define ADDR_DFU_IMG_BASE                   0x1800B0000
#define ADDR_TLBI                           0x100000434
#define ADDR_DC_CIVAC                       0x10000046c
#define ADDR_DMB                            0x100000478
#define ADDR_MAP_RANGE                      0x10000b4ec

/* USB */
#define ADDR_CREATE_DESC                    0x10000d150
#define ADDR_USB_CORE_DO_IO                 0x10000dc98
#define ADDR_HANDLE_INTF_REQ                0x10000dfb8

#define ADDR_CORE_DESC_SERIAL               0x1800805da
#define ADDR_DFU_INTF_HANDLE                0x180088B48
#define ADDR_HS_DESC                        0x180088a30
#define ADDR_FS_DESC                        0x180088a38
#define ADDR_USB_SERIAL                     0x180083cf8

/* Standard */
#define ADDR_DEV_MEMCPY                     0x100010730
#define ADDR_DEV_MEMSET                     0x100010960
#define ADDR_DEV_STRLEN                     0x100010b68

/* System registers */
#define ADDR_WRITE_TTBR0                    0x1000003e4
#define ADDR_DEMOTE_REG                     0x2102bc000

/* Boot */
#define ADDR_TRAMPOLINE                     0x1800ac000
#define LEN_TRAMPOLINE                      0x240
#define TRAMPOLINE_SIZE                     0x4000

#define ADDR_DFU_RETVAL                     0x180088ac8
#define ADDR_DFU_STATUS                     0x180088ac0
#define ADDR_DFU_EVENT                      0x180088af0

/* Misc */
#define ADDR_FUNC_GADGET                    0x10000cc4c
#define ADDR_RANDOM_RET                     0x10000b924
#define ADDR_NOP_GADGET                     0x10000CC6C
#define ADDR_SYNC_ENTRY                     0x1800afc84
#define ADDR_USB_EVENT                      (struct event *)        0x1800838c8

#else
#error "Unsupported checkm8 platform"
#endif

#endif //CHECKM8_TOOL_ADDR_H
