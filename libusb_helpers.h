#ifndef IPWNDFU_REWRITE_C_LIBUSB_HELPERS_H
#define IPWNDFU_REWRITE_C_LIBUSB_HELPERS_H

#include "libusb-1.0/libusb.h"

struct libusb_device_bundle
{
    struct libusb_device_handle *handle;
    struct libusb_device_descriptor descriptor;
};

void get_test_device(libusb_context *usb_ctx, struct libusb_device_bundle *bundle);

void libusb1_async_ctrl_transfer(libusb_device_handle *handle,
                                 unsigned char bmRequestType, unsigned char bRequest,
                                 unsigned short wValue, unsigned short wIndex,
                                 unsigned char *data, unsigned short data_len,
                                 unsigned int timeout);

void libusb1_no_error_ctrl_transfer(libusb_device_handle *handle,
                                    unsigned char bmRequestType, unsigned char bRequest,
                                    unsigned short wValue, unsigned short wIndex,
                                    unsigned char *data, unsigned short data_len,
                                    unsigned int timeout);

void stall(libusb_device_handle *handle);
void leak(libusb_device_handle *handle);
void no_leak(libusb_device_handle *handle);

void usb_req_stall(libusb_device_handle *handle);
void usb_req_leak(libusb_device_handle *handle);
void usb_req_no_leak(libusb_device_handle *handle);

#endif //IPWNDFU_REWRITE_C_LIBUSB_HELPERS_H
