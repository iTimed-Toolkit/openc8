#include <dev_util.h>

#include "bootrom_func.h"
#include "bootrom_types.h"

PAYLOAD_SECTION
void run()
{
    gpio_write(0x1905, 1);
    gpio_write(0x1905, 0);
}


void _start()
{
    run();
}