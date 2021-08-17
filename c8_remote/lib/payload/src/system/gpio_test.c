#include "dev_util.h"
#include "bootrom_func.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE)

PAYLOAD_SECTION
void run()
{
    int i, upper, res;
    int num_zeroes, num_ones;
    struct gpio_test_args *args = GET_GLOBAL(addr_dfu_img_base);

    upper = args->count;
    num_zeroes = args->num_zeroes;
    num_ones = args->num_ones;

    for(i = 0; i < upper; i++)
    {
        for(res = 0; res < num_ones; res++)
            gpio_write(0x1905, 1);

        for(res = 0; res < num_zeroes; res++)
            gpio_write(0x1905, 0);
    }

}

void _start()
{
    run();
}





