#ifndef CHECKM8_TOOL_BRFUNC_TIMING_H
#define CHECKM8_TOOL_BRFUNC_TIMING_H

#include "brfunc_common.h"

#define CLOCK_GATE                  ((BOOTROM_FUNC) ADDR_CLOCK_GATE)
#define SYSTEM_TIME                 ((BOOTROM_FUNC) ADDR_SYSTEM_TIME)
#define TIME_HAS_ELAPSED            ((BOOTROM_FUNC) ADDR_TIME_HAS_ELAPSED)

PAYLOAD_SECTION
void task_sleep(unsigned int usec)
{
    ((BOOTROM_FUNC) ADDR_TASK_SLEEP)(usec);
}

#endif //CHECKM8_TOOL_BRFUNC_TIMING_H
