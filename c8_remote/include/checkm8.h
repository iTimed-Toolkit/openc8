#ifndef CHECKM8_TOOL_CHECKM8_H
#define CHECKM8_TOOL_CHECKM8_H

#include "checkm8_config.h"

typedef enum
{
    CHECKM8_SUCCESS = 0,
    CHECKM8_FAIL_INVARGS,
    CHECKM8_FAIL_NODEV,
    CHECKM8_FAIL_NOEXP,
    CHECKM8_FAIL_NOTDONE,
    CHECKM8_FAIL_XFER,
    CHECKM8_FAIL_NOINST,
    CHECKM8_FAIL_PROT,
    CHECKM8_FAIL_INT
} CHECKM8_STATUS;

#define IS_CHECKM8_FAIL(code) code < 0

#if CHECKM8_PLATFORM == 8010

#define DEV_IDVENDOR    0x05AC
#define DEV_IDPRODUCT   0x1227

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

    struct payload *inst_pl;
    struct data *inst_data;

#ifdef WITH_ARDUINO
    int ard_fd;
#else
    struct libusb_device_bundle *bundle;
#endif
};

struct pwned_device *exploit_device();
void free_device(struct pwned_device *dev);

int demote_device(struct pwned_device *dev);
int fix_heap(struct pwned_device *dev);

int open_device_session(struct pwned_device *dev);
int close_device_session(struct pwned_device *dev);
int is_device_session_open(struct pwned_device *dev);

#endif //CHECKM8_TOOL_CHECKM8_H
