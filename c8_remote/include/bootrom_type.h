#ifndef CHECKM8_TOOL_BOOTROM_TYPE_H
#define CHECKM8_TOOL_BOOTROM_TYPE_H

struct event
{
    unsigned int dat0;
    unsigned int dat1;
    unsigned long long dat2;
    unsigned long long dat3;
} __attribute__ ((packed));

struct aes_constants
{
    unsigned char sbox[16][16];
    unsigned char rc_lookup[11];
    unsigned char mul2[256];
    unsigned char mul3[256];
} __attribute__ ((packed));

struct bern_data
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
