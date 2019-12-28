#ifndef IPWNDFU_REWRITE_C_LIBUSB_HELPERS_H
#define IPWNDFU_REWRITE_C_LIBUSB_HELPERS_H

#include "checkm8.h"
#define LIBUSB_MAX_PACKET_SIZE 0x800

#ifndef WITH_ARDUINO
#include "libusb.h"

struct libusb_device_bundle
{
    struct libusb_context *ctx;
    struct libusb_device *device;
    struct libusb_device_handle *handle;
    struct libusb_device_descriptor *descriptor;
};
#endif

int open_device_session(struct pwned_device *dev);
int close_device_session(struct pwned_device *dev);
int is_device_session_open(struct pwned_device *dev);

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

int ctrl_transfer(struct pwned_device *dev,
                  unsigned char bmRequestType, unsigned char bRequest,
                  unsigned short wValue, unsigned short wIndex,
                  unsigned char *data, unsigned short data_len,
                  unsigned int timeout);

int reset(struct pwned_device *dev);
int serial_descriptor(struct pwned_device *dev, unsigned char *serial_buf, int len);


#endif //IPWNDFU_REWRITE_C_LIBUSB_HELPERS_H
