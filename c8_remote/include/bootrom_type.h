#ifndef CHECKM8_TOOL_BOOTROM_TYPE_H
#define CHECKM8_TOOL_BOOTROM_TYPE_H

struct event
{
    unsigned long long dat0;
    unsigned long long dat1;
    unsigned long long dat2;
} __attribute__ ((packed));

struct aes_sw_bernstein_data
{
    double t[16][256];
    double tsq[16][256];
    double tnum[16][256];

    unsigned long long count;
    double ttotal;

    struct event ev_data;
    struct event ev_done;
} __attribute__ ((packed));

#endif //CHECKM8_TOOL_BOOTROM_TYPE_H
