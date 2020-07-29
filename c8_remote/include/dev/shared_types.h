#ifndef CHECKM8_TOOL_SHARED_TYPES_H
#define CHECKM8_TOOL_SHARED_TYPES_H

#include "experiment_config.h"

#define NUM_GADGETS     32
#define NUM_ADDRESSES   32
#define NUM_ASYNC       4

/* Installation */
struct install_args
{
    int type;
    int len;
    unsigned long long addr;
} __attribute__ ((packed));

/* Utils */
struct rw_args
{
    unsigned long long addr;
    int len;
} __attribute__ ((packed));

struct exec_args
{
    unsigned long long (*addr)();

    unsigned long long args[8];
} __attribute__ ((packed));

struct data_args
{
    enum
    {
        DATA_STATUS,
        DATA_INSTALL,
        DATA_UNINSTALL
    } cmd;

    unsigned long long addr;
    int len;
} __attribute__ ((packed));

struct async_args
{
    enum
    {
        ASYNC_CREATE,
        ASYNC_RUN,
        ASYNC_FREE
    } cmd;

    char name[16];
    unsigned long long func;
    unsigned long long arg;
};

/* Responses */
struct cmd_resp
{
    enum
    {
        CMD_SUCCESS,
        CMD_FAIL_INVALID,
        CMD_FAIL_FULL,
        CMD_FAIL_NOTFOUND,
    } status;

    unsigned long long args[2];
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
} __attribute__ ((packed));

#define DEV_PTR_NULL       -1ull
typedef unsigned long long DEV_PTR_T;

#endif //CHECKM8_TOOL_SHARED_TYPES_H
