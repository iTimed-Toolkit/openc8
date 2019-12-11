#ifndef IPWNDFU_REWRITE_C_LIBUSB_HELPERS_H
#define IPWNDFU_REWRITE_C_LIBUSB_HELPERS_H

#include "checkm8.h"

#define LIBUSB_MAX_PACKET_SIZE 0x800

struct libusb_device_bundle
{
    struct libusb_context *ctx;
    struct libusb_device *device;
    struct libusb_device_handle *handle;
    struct libusb_device_descriptor *descriptor;
};

int get_device_bundle(struct pwned_device *dev);
int release_device_bundle(struct pwned_device *dev);

void libusb1_async_ctrl_transfer(struct pwned_device *dev,
                                 unsigned char bmRequestType, unsigned char bRequest,
                                 unsigned short wValue, unsigned short wIndex,
                                 unsigned char *data, unsigned short data_len,
                                 unsigned int timeout);

void libusb1_no_error_ctrl_transfer(struct pwned_device *dev,
                                    unsigned char bmRequestType, unsigned char bRequest,
                                    unsigned short wValue, unsigned short wIndex,
                                    unsigned char *data, unsigned short data_len,
                                    unsigned int timeout);

void stall(struct pwned_device *dev);
void leak(struct pwned_device *dev);
void no_leak(struct pwned_device *dev);

void usb_req_stall(struct pwned_device *dev);
void usb_req_leak(struct pwned_device *dev);
void usb_req_no_leak(struct pwned_device *dev);

#endif //IPWNDFU_REWRITE_C_LIBUSB_HELPERS_H
