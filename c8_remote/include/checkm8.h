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
#define DEMOTE_REG      0x2102BC000ull

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

    struct payload *inst_pl;
    struct data *inst_data;

#ifdef WITH_ARDUINO
    int ard_fd;
#else
    struct libusb_device_bundle *bundle;
#endif
};

struct dev_cmd_resp
{
    int ret;
    unsigned long long magic;
    unsigned long long retval;
    unsigned char *data;
    int len;
};

struct pwned_device *exploit_device();
void free_device(struct pwned_device *dev);

int demote_device(struct pwned_device *dev);
int fix_heap(struct pwned_device *dev);

int open_device_session(struct pwned_device *dev);
int close_device_session(struct pwned_device *dev);
int is_device_session_open(struct pwned_device *dev);

void free_dev_cmd_resp(struct dev_cmd_resp *resp);

#endif //CHECKM8_TOOL_CHECKM8_H
