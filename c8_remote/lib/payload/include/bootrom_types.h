#ifndef CHECKM8_TOOL_BOOTROM_TYPES_H
#define CHECKM8_TOOL_BOOTROM_TYPES_H

struct event
{
    unsigned int dat0;
    unsigned int dat1;
    unsigned long long dat2;
    unsigned long long dat3;
} __attribute__ ((packed));

struct heap_header
{
    unsigned long long chksum;
    unsigned long long pad[3];

    unsigned long long curr_size;
    unsigned long long curr_free: 1;

    unsigned long long prev_free: 1;
    unsigned long long prev_size: (sizeof(unsigned long long) * 8 - 2);

    unsigned long long pad_start;
    unsigned long long pad_end;
} __attribute__ ((packed));

struct usb_request
{
    unsigned char bmRequestType;
    unsigned char bRequest;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
} __attribute__ ((packed));

#endif //CHECKM8_TOOL_BOOTROM_TYPES_H
