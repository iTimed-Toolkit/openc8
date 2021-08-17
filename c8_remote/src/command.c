#include "tool/command.h"

#include "checkm8.h"
#include "tool/usb_helpers.h"

#include <stdint.h>
#include <stdio.h>

int send_data(struct pwned_device *dev, void *data, long data_len, unsigned int trigger)
{
    checkm8_debug_indent("dfu_send_data(dev = %p, data = %p, data_len = %li)\n", dev, data, data_len);
    long long index = 0, amount;
    int ret;

    while(index < data_len)
    {
        if(data_len - index >= MAX_PACKET_SIZE) amount = MAX_PACKET_SIZE;
        else amount = data_len - index;

        checkm8_debug_indent("\tsending chunk of size %li at boot_index %li\n", amount, index);

        ret = ctrl_transfer(dev, 0x21, 1, 0, 0, &((uint8_t *) data)[index], amount, 0, trigger);
        if(ret > 0) checkm8_debug_indent("\ttransferred %i bytes\n", ret);
        else
        {
            checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, usb_error_name(ret));
            return -CHECKM8_FAIL_XFER;
        }
        index += amount;
    }

    return CHECKM8_SUCCESS;
}

int reset_img_base(struct pwned_device *dev)
{
    checkm8_debug_indent("reset_img_base(dev = %p)\n", dev);
    unsigned char buf[16] = {0};
    int ret = send_data(dev, buf, 16, 0);
    if(IS_CHECKM8_FAIL(ret))
        return ret;

    ret = ctrl_transfer(dev, 0x21, 1, 0, 0, buf, 0, 0, 0);
    if(ret >= 0) checkm8_debug_indent("\ttransferred %i bytes\n", ret);
    else
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, usb_error_name(ret));
        return ret;
    }

    ret = ctrl_transfer(dev, 0xA1, 3, 0, 0, buf, 6, 0, 0);
    if(ret >= 0) checkm8_debug_indent("\ttransferred %i bytes\n", ret);
    else
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, usb_error_name(ret));
        return ret;
    }

    ret = ctrl_transfer(dev, 0xA1, 3, 0, 0, buf, 6, 0, 0);
    if(ret >= 0) checkm8_debug_indent("\ttransferred %i bytes\n", ret);
    else
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, usb_error_name(ret));
        return ret;
    }

    return CHECKM8_SUCCESS;
}

int exit_dfu_mode(struct pwned_device *dev)
{
    int ret;
    uint64_t tmp;

    ret = ctrl_transfer(dev, 0x21, 1, 0, 0, NULL, 0, 0, 0);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, usb_error_name(ret));
        return ret;
    }

    ret = ctrl_transfer(dev, 0xA1, 3, 0, 0, (uint8_t *) &tmp, 8, 0, 0);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, usb_error_name(ret));
        return ret;
    }

    ret = ctrl_transfer(dev, 0xA1, 3, 0, 0, (uint8_t *) &tmp, 8, 0, 0);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, usb_error_name(ret));
        return ret;
    }

    ret = ctrl_transfer(dev, 0xA1, 3, 0, 0, (uint8_t *) &tmp, 8, 0, 0);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\trequest failed with error code %i (%s)\n", ret, usb_error_name(ret));
        return ret;
    }

    reset(dev);
    return CHECKM8_SUCCESS;
}

int command(struct pwned_device *dev,
            short intf,
            void *cmd_args, int cmd_arg_len,
            void *resp_buf, int response_len)
{
    checkm8_debug_indent("command(dev = %p, intf = %i, cmd_args = %p, cmd_arg_len = %i, "
                         "resp_buf = %p, response_len = %i)\n",
                         dev, intf, cmd_args, cmd_arg_len, resp_buf, response_len);

    int close, ret;
    if(is_device_session_open(dev)) close = 0;
    else
    {
        ret = open_device_session(dev, DEV_IDVENDOR, DEV_IDPRODUCT);
        close = 1;

        if(IS_CHECKM8_FAIL(ret))
        {
            checkm8_debug_indent("\tfailed to open device session\n");
            return ret;
        }
    }

    ret = reset_img_base(dev);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to reset DFU total received\n");
        return ret;
    }

    ret = send_data(dev, cmd_args, cmd_arg_len, 0);
    if(IS_CHECKM8_FAIL(ret))
    {
        checkm8_debug_indent("\tfailed to send command arguments\n");
        return ret;
    }

    ret = ctrl_transfer(dev, 0xA1, 2, intf, 0, resp_buf, response_len, 0, 1);
    if(ret >= 0) checkm8_debug_indent("\tfinal request transferred %i bytes\n", ret);
    else
    {
        checkm8_debug_indent("\tfinal request failed with error code %i (%s)\n", ret, usb_error_name(ret));
        return ret;
    }

    if(close) close_device_session(dev);
    return CHECKM8_SUCCESS;
}