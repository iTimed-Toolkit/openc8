#ifndef CHECKM8_TOOL_TYPES_H
#define CHECKM8_TOOL_TYPES_H

#include "experiment_config.h"

struct event
{
    unsigned int dat0;
    unsigned int dat1;
    unsigned long long dat2;
    unsigned long long dat3;
} __attribute__ ((packed));

struct aes_sbox_constants
{
    unsigned char sbox[256];
    unsigned char mul2[256];
    unsigned char mul3[256];
    unsigned char rc_lookup[11];

} __attribute__ ((packed));

struct aes_ttable_constants
{
    unsigned int t0[256];
    unsigned int t1[256];
    unsigned int t2[256];
    unsigned int t3[256];
    unsigned char sbox[256];
    unsigned char rc_lookup[11];
};

struct heap_header
{
    unsigned long long chksum;
    unsigned long long pad[3];

    unsigned long long curr_size;
    unsigned long long curr_free : 1;

    unsigned long long prev_free : 1;
    unsigned long long prev_size : (sizeof(unsigned long long) * 8 - 2);

    unsigned long long pad_start;
    unsigned long long pad_end;
} __attribute__ ((packed));

struct bern_data
{
    unsigned long long t[16][256];
    unsigned long long tsq[16][256];
    unsigned long long tnum[16][256];

    unsigned long long count;
    unsigned long long ttotal;

#ifdef BERNSTEIN_WITH_USB
    struct event ev_data;
    struct event ev_done;
#endif
} __attribute__ ((packed));

#define N_CORR_ENTRIES 1024*256

struct corr_data
{
    struct event ev_cont;

    int num_cutoff;
    unsigned char msg[16];
    unsigned char data[N_CORR_ENTRIES];
};

#define DEV_PTR_NULL       -1ull
typedef unsigned long long DEV_PTR_T;

#endif //CHECKM8_TOOL_TYPES_H
