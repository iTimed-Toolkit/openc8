#ifndef CHECKM8_TOOL_LIBUSB_HELPERS_H
#define CHECKM8_TOOL_LIBUSB_HELPERS_H

#include "checkm8.h"

#define MAX_PACKET_SIZE 0x800

#ifndef WITH_ARDUINO
struct libusb_device_bundle
{
    struct libusb_context *ctx;
    struct libusb_device *device;
    struct libusb_device_handle *handle;
    struct libusb_device_descriptor *descriptor;
};
#endif

int partial_ctrl_transfer(struct pwned_device *dev,
                          unsigned char bmRequestType, unsigned char bRequest,
                          unsigned short wValue, unsigned short wIndex,
                          unsigned char *data, unsigned short data_len,
                          unsigned int timeout);

int no_error_ctrl_transfer(struct pwned_device *dev,
                           unsigned char bmRequestType, unsigned char bRequest,
                           unsigned short wValue, unsigned short wIndex,
                           unsigned char *data, unsigned short data_len,
                           unsigned int timeout);

int no_error_ctrl_transfer_data(struct pwned_device *dev,
                                unsigned char bmRequestType, unsigned char bRequest,
                                unsigned short wValue, unsigned short wIndex,
                                unsigned char *data, unsigned short data_len,
                                unsigned int timeout);

int ctrl_transfer(struct pwned_device *dev,
                  unsigned char bmRequestType, unsigned char bRequest,
                  unsigned short wValue, unsigned short wIndex,
                  unsigned char *data, unsigned short data_len,
                  unsigned int timeout, unsigned int trigger);

int reset(struct pwned_device *dev);
int serial_descriptor(struct pwned_device *dev, unsigned char *serial_buf, int len);

const char *usb_error_name(int ret);

#endif //CHECKM8_TOOL_LIBUSB_HELPERS_H
