#include "checkm8.h"
#include "libusb_helpers.h"

#include "libusb.h"

void dfu_send_data(struct libusb_device_bundle *bundle, unsigned char *data, long data_len)
{
    long index = 0, amount;
    while(index < data_len)
    {
        if(data_len - index >= LIBUSB_MAX_PACKET_SIZE) amount = LIBUSB_MAX_PACKET_SIZE;
        else amount = data_len - index;

        libusb_control_transfer(bundle->handle, 0x21, 1, 0, 0, &data[index], amount, 5000);
        index += amount;
    }
}

static unsigned char nullbuf[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int command(struct pwned_device *dev, void *data, long data_len, void *response, long response_len)
{
    struct libusb_device_bundle bundle;
    int ret = get_device_bundle(&bundle);

    if(IS_CHECKM8_FAIL(ret))
    {
        return ret;
    }

    dfu_send_data(&bundle, nullbuf, 16);
    libusb_control_transfer(bundle.handle, 0x21, 1, 0, 0, NULL, 0, 100);
    libusb_control_transfer(bundle.handle, 0xA1, 3, 0, 0, NULL, 0, 100);
    libusb_control_transfer(bundle.handle, 0xA1, 3, 0, 0, NULL, 6, 100);
    dfu_send_data(&bundle, (unsigned char *) data, data_len);

    if(response_len == 0)
    {
        libusb_control_transfer(bundle.handle, 0xA1, 2, 0xFFFF, 0, response, response_len + 1, 100);
    }
    else
    {
        libusb_control_transfer(bundle.handle, 0xA1, 2, 0xFFFF, 0, response, response_len, 100);
    }

}