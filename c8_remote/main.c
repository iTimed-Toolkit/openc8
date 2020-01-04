#include "checkm8.h"
#include "payload.h"

#include <stdio.h>
#include <stdarg.h>
#include <execinfo.h>
#include <usb_helpers.h>
#include "command.h"

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
    int ret;
    struct dev_cmd_resp *resp;
    struct pwned_device *dev = exploit_device();
    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }


    free_dev_cmd_resp(resp);
    free_device(dev);
}
