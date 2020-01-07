#ifndef CHECKM8_TOOL_CHECKM8_H
#define CHECKM8_TOOL_CHECKM8_H

#include "checkm8_config.h"

#define CHECKM8_SUCCESS         0
#define CHECKM8_FAIL_INVARGS    -1
#define CHECKM8_FAIL_NODEV      -2
#define CHECKM8_FAIL_NOEXP      -3
#define CHECKM8_FAIL_NOTDONE    -4
#define CHECKM8_FAIL_XFER       -5
#define CHECKM8_FAIL_NOINST     -6
#define CHECKM8_FAIL_PROT       -7

#define IS_CHECKM8_FAIL(code) code < 0

#if CHECKM8_PLATFORM == 8010

#define DEV_IDVENDOR    0x05AC
#define DEV_IDPRODUCT   0x1227
#define DFU_IMAGE_BASE  0x1800B0000ull

#else
#error "Unspported checkm8 platform"
#endif

struct pwned_device
{
    enum
    {
        DEV_NORMAL,
        DEV_PWNED
    } status;

    unsigned int idVendor;
    unsigned int idProduct;

    struct payload *installed;

#ifdef WITH_ARDUINO
    int ard_fd;
#else
    struct libusb_device_bundle *bundle;
#endif
};

struct pwned_device *exploit_device();
void free_device(struct pwned_device *dev);

#endif //CHECKM8_TOOL_CHECKM8_H
