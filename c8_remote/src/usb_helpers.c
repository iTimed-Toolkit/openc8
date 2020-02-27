#include "tool/usb_helpers.h"

#ifdef WITH_ARDUINO

#include <termio.h>
#include <fcntl.h>
#include <unistd.h>
#include "ard_protocol.h"

#else

#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#endif

int open_device_session(struct pwned_device *dev)
{
    checkm8_debug_indent("open_device_session(dev = %p)\n", dev);

#ifdef WITH_ARDUINO
    // based on https://github.com/todbot/arduino-serial/blob/master/arduino-serial-lib.c
    struct termios toptions;
    char buf;
    int ard_fd = open(ARDUINO_DEV, O_RDWR | O_NONBLOCK);
    if(ard_fd == -1)
    {
        checkm8_debug_indent("\tfailed to open arduino device %s\n", ARDUINO_DEV);
        return CHECKM8_FAIL_NODEV;
    }

    checkm8_debug_indent("\topened arduino device %s\n", ARDUINO_DEV);
    if(tcgetattr(ard_fd, &toptions) < 0)
    {
        checkm8_debug_indent("\tfailed to get arduino terminal attributes\n");
        close(ard_fd);
        return CHECKM8_FAIL_NODEV;
    }

    speed_t brate;
    switch(ARDUINO_BAUD)
    {
        case 4800:
            brate = B4800;
            break;

        case 9600:
            brate = B9600;
            break;

        case 19200:
            brate = B19200;
            break;

        case 38400:
            brate = B38400;
            break;

        case 57600:
            brate = B57600;
            break;

        case 115200:
            brate = B115200;
            break;

        default:
            brate = B9600;
            break;
    }

    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    toptions.c_cflag &= ~CRTSCTS;

    toptions.c_cflag |= CREAD | CLOCAL;
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY);
    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    toptions.c_oflag &= ~OPOST;

    toptions.c_cc[VMIN] = 0;
    toptions.c_cc[VTIME] = 0;

    tcsetattr(ard_fd, TCSANOW, &toptions);
    if(tcsetattr(ard_fd, TCSAFLUSH, &toptions) < 0)
    {
        checkm8_debug_indent("\tfailed to set terminal attributes");
        close(ard_fd);
        return CHECKM8_FAIL_NODEV;
    }

    checkm8_debug_indent("\tset arduino terminal attributes\n");

    // read a setup verification byte
    while(read(ard_fd, &buf, 1) == 0);
    if(buf == PROT_SUCCESS)
    {
        checkm8_debug_indent("\tarduino successfully setup\n");
        dev->ard_fd = ard_fd;
        return CHECKM8_SUCCESS;
    }
    else if(buf == PROT_FAIL_INITUSB)
    {
        checkm8_debug_indent("\tarduino failed to init USB host shield\n");
        close(ard_fd);
        return CHECKM8_FAIL_NOTDONE;
    }
    else
    {
        checkm8_debug_indent("\tunexpected response %X\n", buf);
        close(ard_fd);
        return CHECKM8_FAIL_PROT;
    }
#else
    int i, usb_dev_count, ret = CHECKM8_FAIL_NODEV;
    libusb_device **usb_device_list = NULL;

    if(dev->bundle->ctx == NULL)
    {
        checkm8_debug_indent("\tbundle ctx is NULL, allocating\n");
        libusb_init(&dev->bundle->ctx);
    }
    else
    {
        if(dev->bundle->descriptor != NULL &&
           dev->bundle->descriptor->idVendor == dev->idVendor &&
           dev->bundle->descriptor->idProduct == dev->idProduct)
        {
            checkm8_debug_indent("\tbundle is already valid\n");
            return CHECKM8_SUCCESS;
        }
    }

    usb_dev_count = libusb_get_device_list(dev->bundle->ctx, &usb_device_list);
    checkm8_debug_indent("\tfound %i USB devices\n", usb_dev_count);

    dev->bundle->device = NULL;
    dev->bundle->handle = NULL;
    dev->bundle->descriptor = malloc(sizeof(struct libusb_device_descriptor));

    for(i = 0; i < usb_dev_count; i++)
    {
        dev->bundle->device = usb_device_list[i];
        libusb_get_device_descriptor(dev->bundle->device, dev->bundle->descriptor);

        if(dev->bundle->descriptor->idVendor == dev->idVendor &&
           dev->bundle->descriptor->idProduct == dev->idProduct)
        {
            checkm8_debug_indent("\tchecking device %i ... match!\n", i);
            ret = CHECKM8_SUCCESS;
            break;
        }

        checkm8_debug_indent("\tchecking device %i ... no match\n", i);
    }

    libusb_free_device_list(usb_device_list, usb_dev_count);
    if(ret == CHECKM8_SUCCESS)
    {
        checkm8_debug_indent("\topening device and returning success\n");
        ret = libusb_open(dev->bundle->device, &dev->bundle->handle);
        if(ret == 0)
        {
            libusb_set_auto_detach_kernel_driver(dev->bundle->handle, 1);
        }
        else
        {
            checkm8_debug_indent("\tfailed to open device\n");
            libusb_exit(dev->bundle->ctx);
            free(dev->bundle->descriptor);
        }

        return ret;
    }
    else
    {
        checkm8_debug_indent("\tcould not find a matching device\n");
        libusb_exit(dev->bundle->ctx);
        free(dev->bundle->descriptor);

        dev->bundle->ctx = NULL;
        dev->bundle->device = NULL;
        dev->bundle->handle = NULL;
        dev->bundle->descriptor = NULL;
    }

    return ret;
#endif
}

int close_device_session(struct pwned_device *dev)
{
    checkm8_debug_indent("close_device_session(dev = %p)\n", dev);

#ifdef WITH_ARDUINO
    int ret = close(dev->ard_fd);
    dev->ard_fd = -1;
    if(ret == -1)
    {
        checkm8_debug_indent("\tfailed to close arduino fd\n");
        return CHECKM8_FAIL_NODEV;
    }

    return CHECKM8_SUCCESS;
#else
    if(dev->bundle->handle != NULL)
    {
        checkm8_debug_indent("\tclosing handle\n");
        libusb_close(dev->bundle->handle);
        dev->bundle->handle = NULL;
    }

    dev->bundle->device = NULL;

    if(dev->bundle->ctx != NULL)
    {
        checkm8_debug_indent("\texiting context\n");;
        libusb_exit(dev->bundle->ctx);
        dev->bundle->ctx = NULL;
    }

    if(dev->bundle->descriptor != NULL)
    {
        checkm8_debug_indent("\tfreeing device descriptor\n");
        free(dev->bundle->descriptor);
        dev->bundle->descriptor = NULL;
    }

    return CHECKM8_SUCCESS;
#endif
}

int is_device_session_open(struct pwned_device *dev)
{
#ifdef WITH_ARDUINO
    return dev->ard_fd != -1;
#else
    return dev->bundle->ctx != NULL && dev->bundle->device != NULL &&
           dev->bundle->handle != NULL && dev->bundle->descriptor != NULL;
#endif
}

#ifdef WITH_ARDUINO

void ard_read(struct pwned_device *dev, unsigned char *target, int nbytes)
{
    int index = 0, amount;
    while(index < nbytes)
    {
        amount = read(dev->ard_fd, &target[index], nbytes - index);
        if(amount == 0) usleep(5000);
        else index += amount;
    }
}

#endif


int partial_ctrl_transfer(struct pwned_device *dev,
                          unsigned char bmRequestType, unsigned char bRequest,
                          unsigned short wValue, unsigned short wIndex,
                          unsigned char *data, unsigned short data_len,
                          unsigned int timeout)
{
    checkm8_debug_indent(
            "partial_ctrl_transfer(dev = %p, bmRequestType = %i, bRequest = %i, wValue = %i, wIndex = %i, data = %p, data_len = %i, timeout = %i)\n",
            dev, bmRequestType, bRequest, wValue, wIndex, data, data_len, timeout);

#ifdef WITH_ARDUINO
    char buf;
    struct usb_xfer_args args;
    args.bmRequestType = bmRequestType;
    args.bRequest = bRequest;
    args.wValue = wValue;
    args.wIndex = wIndex;
    args.data_len = data_len;

    checkm8_debug_indent("\tsending data to arduino\n");
    write(dev->ard_fd, &PROT_PARTIAL_CTRL_XFER, 1);
    write(dev->ard_fd, &args, sizeof(struct usb_xfer_args));

    ard_read(dev, (unsigned char *) &buf, 1);
    if(buf == PROT_ACK)
    {
        checkm8_debug_indent("\treceived ack\n");

        ard_read(dev, (unsigned char *) &buf, 1);
        if(buf == PROT_SUCCESS)
        {
            checkm8_debug_indent("\tsuccess\n");
            return CHECKM8_SUCCESS;
        }
        else if(buf == PROT_FAIL_USB)
        {
            ard_read(dev, (unsigned char *) &buf, 1);

            checkm8_debug_indent("\trequest failed with error %X\n", buf);
            return CHECKM8_FAIL_XFER;
        }
        else
        {
            checkm8_debug_indent("\tunexpected response %X\n", buf);
            return CHECKM8_FAIL_PROT;
        }
    }
    else
    {
        checkm8_debug_indent("\tno ack received (got %x)\n", buf);
        return CHECKM8_FAIL_PROT;
    }
#else
    struct timeval start, end;
    unsigned char usb_transfer_buf[8 + data_len];
    int ret;

    gettimeofday(&start, NULL);

    struct libusb_transfer *usb_transfer = libusb_alloc_transfer(0);
    libusb_fill_control_setup(usb_transfer_buf, bmRequestType, bRequest, wValue, wIndex, data_len);
    memcpy(&usb_transfer_buf[8], data, data_len);
    libusb_fill_control_transfer(usb_transfer, dev->bundle->handle, usb_transfer_buf, NULL, NULL, 1);

    checkm8_debug_indent("\tsubmiting urb\n");
    ret = libusb_submit_transfer(usb_transfer);
    if(ret != 0)
    {
        checkm8_debug_indent("\tfailed to submit async USB transfer: %s\n", libusb_error_name(ret));
        libusb_free_transfer(usb_transfer);
        return CHECKM8_FAIL_XFER;
    }

    while(1)
    {
        gettimeofday(&end, NULL);
        if((1000000 * end.tv_sec + end.tv_usec) - (1000000 * end.tv_sec + start.tv_usec) > timeout)
        {
            ret = libusb_cancel_transfer(usb_transfer);
            if(ret != 0)
            {
                checkm8_debug_indent("\tfailed to cancel async USB transfer: %s\n", libusb_error_name(ret));
                return CHECKM8_FAIL_XFER;
            }

            return CHECKM8_SUCCESS;
        }
    }
#endif
}

int no_error_ctrl_transfer(struct pwned_device *dev,
                           unsigned char bmRequestType, unsigned char bRequest,
                           unsigned short wValue, unsigned short wIndex,
                           unsigned char *data, unsigned short data_len,
                           unsigned int timeout)
{
    checkm8_debug_indent(
            "no_error_ctrl_transfer(dev = %p, bmRequestType = %i, bRequest = %i, wValue = %i, wIndex = %i, data = %p, data_len = %i, timeout = %i)\n",
            dev, bmRequestType, bRequest, wValue, wIndex, data, data_len, timeout);

#ifdef WITH_ARDUINO
    char buf;
    struct usb_xfer_args args;
    args.bmRequestType = bmRequestType;
    args.bRequest = bRequest;
    args.wValue = wValue;
    args.wIndex = wIndex;
    args.data_len = data_len;

    checkm8_debug_indent("\tsending data to arduino\n");
    write(dev->ard_fd, &PROT_NO_ERROR_CTRL_XFER, 1);
    write(dev->ard_fd, &args, sizeof(struct usb_xfer_args));

    ard_read(dev, (unsigned char *) &buf, 1);
    if(buf == PROT_ACK)
    {
        checkm8_debug_indent("\treceived ack\n");
        do
        {
            if(buf == PROT_FAIL_USB)
            {
                ard_read(dev, (unsigned char *) &buf, 1);
                checkm8_debug_indent("\treceived error %X but ignoring\n", buf);
            }

            ard_read(dev, (unsigned char *) &buf, 1);
        } while(buf != PROT_SUCCESS);

        checkm8_debug_indent("\tsuccess\n");
        return CHECKM8_SUCCESS;
    }
    else
    {
        checkm8_debug_indent("\tno ack received (got %x)\n", buf);
        return CHECKM8_FAIL_PROT;
    }
#else
    int ret;
    unsigned char recipient = bmRequestType & 3u;
    unsigned char rqtype = bmRequestType & (3u << 5u);
    if(recipient == 1 && rqtype == (2u << 5u))
    {
        unsigned short interface = wIndex & 0xFFu;
        ret = libusb_claim_interface(dev->bundle->handle, interface);
        if(ret > 0)
        {
            checkm8_debug_indent("\tfailed to claim interface: %s\n", libusb_error_name(ret));
            return CHECKM8_FAIL_XFER;
        }
    }

    ret = libusb_control_transfer(dev->bundle->handle, bmRequestType, bRequest, wValue, wIndex, data, data_len,
                                  timeout);
    checkm8_debug_indent("\tgot error %s but ignoring\n", libusb_error_name(ret));
    return CHECKM8_SUCCESS;
#endif
}

int no_error_ctrl_transfer_data(struct pwned_device *dev,
                                unsigned char bmRequestType, unsigned char bRequest,
                                unsigned short wValue, unsigned short wIndex,
                                unsigned char *data, unsigned short data_len,
                                unsigned int timeout)
{
    checkm8_debug_indent(
            "no_error_ctrl_transfer_data(dev = %p, bmRequestType = %i, bRequest = %i, wValue = %i, wIndex = %i, data = %p, data_len = %i, timeout = %i)\n",
            dev, bmRequestType, bRequest, wValue, wIndex, data, data_len, timeout);
#ifdef WITH_ARDUINO
    int amount, index = 0;
    char buf;
    struct usb_xfer_args args;
    args.bmRequestType = bmRequestType;
    args.bRequest = bRequest;
    args.wValue = wValue;
    args.wIndex = wIndex;
    args.data_len = data_len;

    checkm8_debug_indent("\tsending data to arduino\n");
    write(dev->ard_fd, &PROT_NO_ERROR_CTRL_XFER_DATA, 1);
    write(dev->ard_fd, &args, sizeof(struct usb_xfer_args));

    ard_read(dev, (unsigned char *) &buf, 1);
    if(buf == PROT_ACK)
    {
        checkm8_debug_indent("\treceived argument ack\n");
        while(index < data_len)
        {
            amount = 0;
            ard_read(dev, (unsigned char *) &amount, 2);
            checkm8_debug_indent("\twriting data chunk of size %i\n", amount);
            write(dev->ard_fd, &data[index], amount);

            index += amount;
        }

        ard_read(dev, (unsigned char *) &buf, 1);
        if(buf == PROT_SUCCESS)
        {
            checkm8_debug_indent("\tsuccess\n");
            return CHECKM8_SUCCESS;
        }
        else
        {
            checkm8_debug_indent("\tunexpected response %x\n", buf);
            return CHECKM8_FAIL_PROT;
        }
    }
    else
    {
        checkm8_debug_indent("\tno ack received (got %x)\n", buf);
        return CHECKM8_FAIL_PROT;
    }
#else
    return no_error_ctrl_transfer(dev, bmRequestType, bRequest, wValue, wIndex, data, data_len, timeout);
#endif
}

int ctrl_transfer(struct pwned_device *dev,
                  unsigned char bmRequestType, unsigned char bRequest,
                  unsigned short wValue, unsigned short wIndex,
                  unsigned char *data, unsigned short data_len,
                  unsigned int timeout, unsigned int trigger)
{
    checkm8_debug_indent(
            "ctrl_transfer(dev = %p, bmRequestType = %X, bRequest = %X, wValue = %i, wIndex = %i, data = %p, data_len = %i, timeout = %i)\n",
            dev, bmRequestType, bRequest, wValue, wIndex, data, data_len, timeout);

#ifdef WITH_ARDUINO
    unsigned int amount, index, size;
    char buf;
    struct usb_xfer_args args;
    args.bmRequestType = bmRequestType;
    args.bRequest = bRequest;
    args.wValue = wValue;
    args.wIndex = wIndex;
    args.data_len = data_len;
    args.trigger = trigger;

    checkm8_debug_indent("\tsending data to arduino\n");
    write(dev->ard_fd, &PROT_CTRL_XFER, 1);
    write(dev->ard_fd, &args, sizeof(struct usb_xfer_args));

    ard_read(dev, (unsigned char *) &buf, 1);
    if(buf == PROT_ACK)
    {
        checkm8_debug_indent("\treceived argument ack\n");
        if(bmRequestType & 0x80u)
        {
            amount = 0;
            while(amount < data_len)
            {
                // get the size of this chunk
                size = 0;
                ard_read(dev, (unsigned char *) &size, 2);
                if(size > ARD_BUF_SIZE)
                {
                    checkm8_debug_indent("\treceived bad chunk size %i\n", size);
                    return CHECKM8_FAIL_XFER;
                }

                checkm8_debug_indent("\treceiving data chunk of size %i\n", size);

                ard_read(dev, (unsigned char *) &data[amount], size);
                amount += size;
            }
        }
        else
        {
            index = 0;
            while(index < data_len)
            {
                amount = 0;
                ard_read(dev, (unsigned char *) &amount, 2);
                checkm8_debug_indent("\twriting data chunk of size %i\n", amount);

                write(dev->ard_fd, &data[index], amount);
                index += amount;
            }
        }

        ard_read(dev, (unsigned char *) &buf, 1);
        if(buf == PROT_SUCCESS)
        {
            checkm8_debug_indent("\tsuccess\n");
            return data_len;
        }
        else
        {
            checkm8_debug_indent("\tunexpected response %x\n", buf);
            return CHECKM8_FAIL_PROT;
        }
    }
    else
    {
        checkm8_debug_indent("\tno ack received (got %x)\n", buf);
        return CHECKM8_FAIL_PROT;
    }
#else
    return libusb_control_transfer(dev->bundle->handle,
                                   bmRequestType, bRequest,
                                   wValue, wIndex,
                                   data, data_len,
                                   timeout);
#endif
}

int reset(struct pwned_device *dev)
{
    checkm8_debug_indent("reset(dev = %p)\n", dev);
#ifdef WITH_ARDUINO
    char buf;
    write(dev->ard_fd, &PROT_RESET, 1);

    ard_read(dev, (unsigned char *) &buf, 1);
    if(buf == PROT_ACK)
    {
        checkm8_debug_indent("\treceived ack\n");

        ard_read(dev, (unsigned char *) &buf, 1);
        if(buf == PROT_SUCCESS)
        {
            checkm8_debug_indent("\tsuccess\n");
            return CHECKM8_SUCCESS;
        }
        else
        {
            checkm8_debug_indent("\tunexpected response %X\n", buf);
            return CHECKM8_FAIL_PROT;
        }
    }
    else
    {
        checkm8_debug_indent("\tno ack received (got %x)\n", buf);
        return CHECKM8_FAIL_PROT;
    }
#else
    return libusb_reset_device(dev->bundle->handle);
#endif
}

int serial_descriptor(struct pwned_device *dev, unsigned char *serial_buf, int len)
{
    checkm8_debug_indent("serial_descriptor(dev = %p, serial_buf = %p, len = %i)\n", dev, serial_buf, len);

#ifdef WITH_ARDUINO
    char buf;
    struct serial_desc_args args;
    args.dev_idVendor = dev->idVendor;
    args.dev_idProduct = dev->idProduct;
    args.len = len;

    checkm8_debug_indent("\tsending data to arduino\n");
    write(dev->ard_fd, &PROT_SERIAL_DESC, 1);
    write(dev->ard_fd, &args, sizeof(struct serial_desc_args));

    ard_read(dev, (unsigned char *) &buf, 1);
    if(buf == PROT_ACK)
    {
        checkm8_debug_indent("\treceived ack\n");
        while(read(dev->ard_fd, &buf, 1) == 0);
        if(buf == PROT_FAIL_NODEV)
        {
            checkm8_debug_indent("\tno device attached\n");
            return CHECKM8_FAIL_NODEV;
        }
        else if(buf == PROT_FAIL_WRONGDEV)
        {
            checkm8_debug_indent("\twrong device attached\n");
            return CHECKM8_FAIL_NODEV;
        }
        else if(buf == PROT_SUCCESS)
        {
            checkm8_debug_indent("\tsuccess, reading serial descriptor\n");
            ard_read(dev, serial_buf, len);
            return CHECKM8_SUCCESS;
        }
        else
        {
            checkm8_debug_indent("\tunexpected response %X\n", buf);
            return CHECKM8_FAIL_PROT;
        }
    }
    else
    {
        checkm8_debug_indent("\tno ack received (got %x)\n", buf);
        return CHECKM8_FAIL_PROT;
    }
#else
    struct libusb_device_handle *handle = dev->bundle->handle;
    struct libusb_device_descriptor *desc = dev->bundle->descriptor;

    libusb_get_string_descriptor_ascii(handle, desc->iSerialNumber, serial_buf, len);
    return CHECKM8_SUCCESS;
#endif
}

const char *usb_error_name(int code)
{
#ifdef WITH_ARDUINO
    switch(code)
    {
        case CHECKM8_SUCCESS:
            return "CHECKM8_SUCCESS";

        case CHECKM8_FAIL_INVARGS:
            return "CHECKM8_FAIL_INVARGS";

        case CHECKM8_FAIL_NODEV:
            return "CHECKM8_FAIL_NODEV";

        case CHECKM8_FAIL_NOEXP:
            return "CHECKM8_FAIL_NOEXP";

        case CHECKM8_FAIL_NOTDONE:
            return "CHECKM8_FAIL_NOTDONE";

        case CHECKM8_FAIL_XFER:
            return "CHECKM8_FAIL_XFER";

        case CHECKM8_FAIL_NOINST:
            return "CHECKM8_FAIL_NOINST";

        case CHECKM8_FAIL_PROT:
            return "CHECKM8_FAIL_PROT";

        default:
            return "UNKNOWN";
    }
#else
    return libusb_error_name(code);
#endif
}