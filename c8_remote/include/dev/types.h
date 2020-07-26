#ifndef CHECKM8_TOOL_TYPES_H
#define CHECKM8_TOOL_TYPES_H

#include "experiment_config.h"

#define INSTALL_NAME_SZ 16

struct install_info
{
    char name[INSTALL_NAME_SZ];

    long long len;
    void * (*handler)(void *);
};

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

struct bern_data
{
#ifdef BERNSTEIN_COLLECT1
    unsigned long long t1[128][2];
    unsigned long long tsq1[128][2];
    unsigned long long tnum1[128][2];

    unsigned long long count1;
    unsigned long long ttotal1;
#endif

#ifdef BERNSTEIN_COLLECT2
    unsigned long long t2[64][4];
    unsigned long long tsq2[64][4];
    unsigned long long tnum2[64][4];

    unsigned long long count2;
    unsigned long long ttotal2;
#endif

#ifdef BERNSTEIN_COLLECT4
    unsigned long long t4[32][16];
    unsigned long long tsq4[32][16];
    unsigned long long tnum4[32][16];

    unsigned long long count4;
    unsigned long long ttotal4;
#endif

#ifdef BERNSTEIN_COLLECT8
    unsigned long long t8[16][256];
    unsigned long long tsq8[16][256];
    unsigned long long tnum8[16][256];

    unsigned long long count8;
    unsigned long long ttotal8;
#endif

#ifdef BERNSTEIN_WITH_USB
    struct event ev_data;
#endif
    struct event ev_done;
} __attribute__ ((packed));

#define DEV_PTR_NULL       -1ull
typedef unsigned long long DEV_PTR_T;

#endif //CHECKM8_TOOL_TYPES_H
