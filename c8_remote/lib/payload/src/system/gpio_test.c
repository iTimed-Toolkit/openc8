#include "dev_util.h"
#include "bootrom_func.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE)
GLOBAL_PTR(addr_get_force_dfu, 0x1000082b0)
GLOBAL_PTR(addr_gpio_write, 0x100001e78)

PAYLOAD_SECTION
void run()
{
    int i, upper, res;
    int num_zeroes = 0, num_ones = 0;
    struct gpio_test_args *args = GET_GLOBAL(addr_dfu_img_base), *resp = args;
//    int (*get_force_dfu)(void) = GET_GLOBAL(addr_get_force_dfu);
//
//    upper = args->count;
//    for(i = 0; i < upper; i++)
//    {
//        res = get_force_dfu();
//        if(res)
//            num_ones++;
//        else
//            num_zeroes++;
//    }
//
//    resp->num_zeroes = num_zeroes;
//    resp->num_ones = num_ones;

    void (*gpio_write)(int, int) = GET_GLOBAL(addr_gpio_write);
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