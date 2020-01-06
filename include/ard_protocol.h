#ifndef CHECKM8_TOOL_ARD_PROTOCOL_H
#define CHECKM8_TOOL_ARD_PROTOCOL_H

#define ARD_BUF_SIZE 512

static const char PROT_PARTIAL_CTRL_XFER       = 'P';
static const char PROT_NO_ERROR_CTRL_XFER      = 'N';
static const char PROT_NO_ERROR_CTRL_XFER_DATA = 'M';
static const char PROT_CTRL_XFER               = 'C';
static const char PROT_RESET                   = 'R';
static const char PROT_SERIAL_DESC             = 'S';

static const char PROT_SUCCESS       = '\x00';
static const char PROT_ACK           = '\x01';
static const char PROT_FAIL_BADCMD   = '\xFF';
static const char PROT_FAIL_NODEV    = '\xFE';
static const char PROT_FAIL_WRONGDEV = '\xFD';
static const char PROT_FAIL_USB      = '\xFC';
static const char PROT_FAIL_TOOBIG   = '\xFB';

static const char PROT_FAIL_INITUSB  = '\xEF';

struct usb_xfer_args
{
    unsigned char bmRequestType;
    unsigned char bRequest;
    unsigned short wValue;
    unsigned short wIndex;

    unsigned short data_len;
    unsigned char trigger;
} __attribute__ ((packed));

struct serial_desc_args
{
    unsigned short dev_idVendor;
    unsigned short dev_idProduct;
    unsigned char len;
} __attribute__ ((packed));

#endif //CHECKM8_TOOL_ARD_PROTOCOL_H
