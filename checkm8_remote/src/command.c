#include "command.h"

#include "checkm8.h"
#include "libusb_helpers.h"
#include "libusb.h"

#include "stdlib.h"

void dfu_send_data(struct pwned_device *dev, unsigned char *data, long data_len)
{
    checkm8_debug("dfu_send_data(dev = %p, data = %p, data_len = %li)\n", dev, data, data_len);
    long index = 0, amount;
    int ret;

    while(index < data_len)
    {
        if(data_len - index >= LIBUSB_MAX_PACKET_SIZE) amount = LIBUSB_MAX_PACKET_SIZE;
        else amount = data_len - index;

        checkm8_debug("sending chunk of size %li at index %li\n", amount, index);
        ret = libusb_control_transfer(dev->bundle->handle, 0x21, 1, 0, 0, &data[index], amount, 5000);
        checkm8_debug("transferred %i bytes\n", ret);
        index += amount;
    }
}

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

static unsigned char nullbuf[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int command(struct pwned_device *dev, struct command_args *args, struct command_args *resp, long response_len)
{
    checkm8_debug("command(dev = %p, args = %p, resp = %p, response_len = %li)\n", dev, args, resp, response_len);
    if(!is_device_bundle_open(dev)) return CHECKM8_FAIL_NODEV;

    int ret;
    dfu_send_data(dev, nullbuf, 16);
    ret = libusb_control_transfer(dev->bundle->handle, 0x21, 1, 0, 0, nullbuf, 0, 100);
    checkm8_debug("got return code %i (%s) from control transfer\n", ret, libusb_error_name(ret));

    ret = libusb_control_transfer(dev->bundle->handle, 0xA1, 3, 0, 0, nullbuf, 6, 100);
    checkm8_debug("got return code %i (%s) from control transfer\n", ret, libusb_error_name(ret));

    ret = libusb_control_transfer(dev->bundle->handle, 0xA1, 3, 0, 0, nullbuf, 6, 100);
    checkm8_debug("got return code %i (%s) from control transfer\n", ret, libusb_error_name(ret));

    dfu_send_data(dev, (unsigned char *) args, args->len);
    if(response_len == 0)
    {
        ret = libusb_control_transfer(dev->bundle->handle, 0xA1, 2, 0xFFFF, 0, (unsigned char *) resp, response_len + 1, 100);
        checkm8_debug("got return code %i (%s) from final response transfer\n", ret, libusb_error_name(ret));
    }
    else
    {
        ret = libusb_control_transfer(dev->bundle->handle, 0xA1, 2, 0xFFFF, 0, (unsigned char *) resp, response_len, 100);
        checkm8_debug("got return code %i (%s) from control transfer\n", ret, libusb_error_name(ret));
    }

    checkm8_debug("got response magic %X\n", resp->magic);
    return CHECKM8_SUCCESS;
}

#define EXEC_MAGIC 0x6365786563657865ul // 'execexec'[::-1]
#define MEMC_MAGIC 0x636d656d636d656dul // 'memcmemc'[::-1]
#define MEMS_MAGIC 0x736d656d736d656dul // 'memsmems'[::-1]
#define DONE_MAGIC 0x656e6f64656e6f64ul // 'donedone'[::-1]

int dev_memset(struct pwned_device *dev, long addr, unsigned char c, long len)
{
    checkm8_debug("dev_memset(dev = %p, addr = %lx, c = %x, len = %li)\n", dev, addr, c, len);
    int ret;
    struct command_args *cmd_args, *cmd_resp;
    cmd_args = calloc(1, sizeof(struct command_args));
    cmd_resp = calloc(1, sizeof(struct command_args));

    checkm8_debug("cmd_args = %p, cmd_resp = %p\n", cmd_args, cmd_resp);
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
    checkm8_debug("dev_memset(dev = %p, dest = %lx, src = %lx, len = %li)\n", dev, dest, src, len);
    int ret;
    struct command_args *cmd_args, *cmd_resp;
    cmd_args = calloc(1, sizeof(struct command_args));
    cmd_resp = calloc(1, sizeof(struct command_args));

    checkm8_debug("cmd_args = %p, cmd_resp = %p\n", cmd_args, cmd_resp);
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
    checkm8_debug("dev_exec(dev = %p, response_len = %l, nargs = %i, args = %p\n", dev, response_len, nargs, args);
    if(nargs > 7)
    {
        checkm8_debug("too many args\n");
        return CHECKM8_FAIL_INVARGS;
    }

    int ret;
    unsigned long *argbase;
    struct command_args *cmd_args, *cmd_resp;
    cmd_args = calloc(1, sizeof(struct command_args));
    cmd_resp = calloc(1, sizeof(struct command_args));

    checkm8_debug("cmd_args = %p, cmd_resp = %p\n", cmd_args, cmd_resp);
    checkm8_debug("copying args: ");

    cmd_args->magic = EXEC_MAGIC;
    argbase = &cmd_args->arg1;
    for(ret = 0; ret < nargs; ret++)
    {
        checkm8_debug("%lx ", args[ret]);
        argbase[ret] = args[ret];
    }
    checkm8_debug("\n");

    ret = command(dev, cmd_args, cmd_resp, 16 + response_len);
    if(ret == CHECKM8_SUCCESS && cmd_resp->magic != DONE_MAGIC) return CHECKM8_FAIL_NOTDONE;
    else return ret;
}