#ifndef CHECKM8_TOOL_TYPES_H
#define CHECKM8_TOOL_TYPES_H

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
    unsigned char mul2[256];
    unsigned char mul3[256];
    unsigned char rc_lookup[11];
} __attribute__ ((packed));

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
    double t[16][256];
    double tsq[16][256];
    double tnum[16][256];

    unsigned long long count;
    double ttotal;

    struct event ev_data;
    struct event ev_done;
} __attribute__ ((packed));

#define N_CORR_ENTRIES 1024*256

struct corr_data
{
    struct event ev_cont;

    int num_cutoff;
    unsigned char msg[16];
    unsigned char data[N_CORR_ENTRIES];
};

#endif //CHECKM8_TOOL_TYPES_H
