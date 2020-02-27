#ifndef CHECKM8_TOOL_ADDR_H
#define CHECKM8_TOOL_ADDR_H

#include "checkm8_config.h"
#include "types.h"

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

#define ADDR_EVENT_NEW                      0x10000aed4
#define ADDR_EVENT_NOTIFY                   0x10000aee8
#define ADDR_EVENT_WAIT                     0x10000af3c
#define ADDR_EVENT_TRY                      0x10000af7c

/* Heap */
#define ADDR_CALC_CHKSUM                    0x10000ee20
#define ADDR_CHECK_BLOCK_CKSUM              0x10000f138
#define ADDR_CHECK_ALL_CHKSUMS              0x10000f8b4

#define ADDR_DEV_MALLOC                     0x10000efe0
#define ADDR_DEV_MEMALIGN                   0x10000f380
#define ADDR_DEV_FREE                       0x10000f1b0

/* Misc */
#define ADDR_RANDOM_RET                     0x10000b924
#define ADDR_SYNC_ENTRY                     0x1800afc84

#define ADDR_DFU_RETVAL                     (int *)                 0x180088ac8
#define ADDR_DFU_STATUS                     (unsigned char *)       0x180088ac0
#define ADDR_DFU_EVENT                      (struct event *)        0x180088af0
#define ADDR_USB_EVENT                      (struct event *)        0x1800838c8

#else
#error "Unsupported checkm8 platform"
#endif

#endif //CHECKM8_TOOL_ADDR_H
