#include <string.h>
#include <stdio.h>

#include "libusb_helpers.h"
#include "checkm8.h"

#define EXEC_MAGIC 0x6365786563657865
#define DONE_MAGIC 0x656e6f64656e6f64
#define MEMC_MAGIC 0x636d656d636d656d
#define MEMS_MAGIC 0x736d656d736d656d

int command(unsigned char *request_data, int request_len, unsigned char *response_buf, int response_len)
{
    libusb_context *usb_ctx = NULL;
    struct libusb_device_bundle bundle;

    libusb_init(&usb_ctx);
    get_test_device(usb_ctx, &bundle);

    unsigned char nullbuf[16];
    memset(nullbuf, '\0', 16);

    libusb_control_transfer(bundle.handle, 0x21, 1, 0, 0, nullbuf, 16, 5000);
    libusb_control_transfer(bundle.handle, 0x21, 1, 0, 0, nullbuf, 0, 100);
    libusb_control_transfer(bundle.handle, 0xA1, 3, 0, 0, nullbuf, 6, 100);
    libusb_control_transfer(bundle.handle, 0xA1, 3, 0, 0, nullbuf, 6, 100);
    libusb_control_transfer(bundle.handle, 0x21, 1, 0, 0, request_data, request_len, 5000);

    if(response_len == 0)
    {
        libusb_control_transfer(bundle.handle, 0xA1, 2, 0xFFFF, 0, response_buf, 1, 5000);
        return 0;
    }
    else
    {
        libusb_control_transfer(bundle.handle, 0xA1, 2, 0xFFFF, 0, response_buf, request_len, 5000);
        return 0;
    }
}


int execute(unsigned long *args, int nargs, unsigned char *response_buf, int response_len)
{
    unsigned char cmd_buf[8 * (nargs + 1)];
    unsigned long exec = EXEC_MAGIC;

    memcpy(cmd_buf, &exec, 8);
    memcpy(&cmd_buf[8], args, 8 * nargs);
    return command(cmd_buf, 8 * (nargs + 1), response_buf, response_len);
}


int aes(unsigned char *source, unsigned char *target, int encrypt, int key)
{
    unsigned long args[10];
    args[0] = 0x10000C8F4;  // AES crypto command
    args[1] = encrypt;
    args[2] = 0x1800b0048;  // cmd_data_address(7)
    args[3] = 0x1800B0010;  // cmd_data_address(0)
    args[4] = 128;          // length of the data
    args[5] = key;
    args[6] = 0;
    args[7] = 0;
    memcpy(&args[8], source, 16);

    unsigned char response[32];
    int ret = execute(args, 10, response, 32);

    memcpy(target, &response[16], 16);
    for(int i = 0; i < 16; i++)
    {
        printf("%02x", target[i]);
    }
    return ret;
}