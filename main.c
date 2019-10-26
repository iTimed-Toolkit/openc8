#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libusb_helpers.h"

int main()
{
    int ret;
    libusb_context *usb_ctx = NULL;
    libusb_init(&usb_ctx);

    struct libusb_device_bundle usb_bundle;
    get_test_device(usb_ctx, &usb_bundle);

    if(usb_bundle.handle == NULL)
    {
        libusb_exit(usb_ctx);
        printf("Could not find device\n");
        return 1;
    }

    struct libusb_device_handle *usb_handle = usb_bundle.handle;
    struct libusb_device_descriptor usb_desc = usb_bundle.descriptor;

    ret = libusb_set_auto_detach_kernel_driver(usb_handle, 1);
    if(ret > 0)
    {
        printf("%s\n", libusb_error_name(ret));
        exit(1);
    }

    unsigned char usb_serial_buf[128];
    unsigned char usb_data_buf[2048];
    unsigned char usb_transfer_buf[2048];

    libusb_get_string_descriptor_ascii(usb_handle, usb_desc.iSerialNumber, usb_serial_buf, sizeof(usb_serial_buf));
    printf("Found device with serial %s\n", usb_serial_buf);

    // begin the USB magic section
    unsigned int i;

    stall(usb_handle);
    for(i = 0; i < 5; i++)
    {
        no_leak(usb_handle);
    }
    usb_req_leak(usb_handle);
    no_leak(usb_handle);

//    libusb_close(usb_handle);
//    libusb_exit(usb_ctx);
//
//    usb_bundle.handle = NULL;
//
//    // section 2
//    libusb_init(&usb_ctx);
//    get_test_device(usb_ctx, &usb_bundle);
//    if(usb_bundle.handle == NULL)
//    {
//        libusb_exit(usb_ctx);
//        printf("Could not find device\n");
//        return 1;
//    }


    return 0;
}