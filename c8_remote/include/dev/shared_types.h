#ifndef CHECKM8_TOOL_SHARED_TYPES_H
#define CHECKM8_TOOL_SHARED_TYPES_H

#include "experiment_config.h"

#define NUM_GADGETS     32
#define NUM_ADDRESSES   32
#define NUM_ASYNC       4

#define EXECUTABLE_SRAM(addr)   addr + 0x2000000
#define WRITEABLE_SRAM(addr)    addr - 0x2000000

#define WRITEABLE_ROM(addr)     addr + 0x2000000

#define CR_EXECUTABLE_SRAM(addr)    addr - 0x40000000
#define CR_WRITEABLE_SRAM(addr)     addr - 0x3E000000

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

/* Crypto */
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

struct hw_aes_args
{
    enum
    {
        DIR_ENC = 0x0,
        DIR_DEC = 0x1
    } dir;

    enum
    {
        MODE_ECB = 0x00,
        MODE_CBC = 0x10
    } mode;

    enum
    {
        KEY_USER = 0x000,
        KEY_UID = 0x100,
        KEY_GID0 = 0x200,
        KEY_GID1 = 0x201
    } type;

    enum
    {
        SIZE_128 = 0x00000000,
        SIZE_192 = 0x10000000,
        SIZE_256 = 0x20000000
    } size;

    unsigned char msg[16];
    unsigned char key[16];
} __attribute__ ((packed));

typedef enum
{
    HX_DIR_DEC = 0,
    HX_DIR_ENC
} hx_dir_t;

typedef enum
{
    HX_MODE_ECB = 0,
    HX_MODE_CBC
} hx_mode_t;

typedef enum
{
    HX_KEYSIZE_128 = 0,
    HX_KEYSIZE_192,
    HX_KEYSIZE_256
} hx_keysize_t;

typedef enum
{
    HX_KEY_NONE = -1,
    HX_KEY_USER,
    HX_KEY_UID,
    HX_KEY_GID0,
    HX_KEY_GID1
} hx_keytype_t;

struct hx_aes_ctx
{
    hx_dir_t encdir;
    hx_mode_t encmode;
    hx_keysize_t keysize;
    hx_keytype_t keytype;

    unsigned char msg[32];
    unsigned char key[32];
    unsigned char iv[32];
} __attribute__ ((packed));

struct gpio_test_args
{
    int count;

    int num_zeroes;
    int num_ones;
} __attribute__ ((packed));

#define DEV_PTR_NULL       -1ull
typedef unsigned long long DEV_PTR_T;

#endif //CHECKM8_TOOL_SHARED_TYPES_H
