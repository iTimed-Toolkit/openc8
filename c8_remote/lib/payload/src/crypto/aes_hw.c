#include "bootrom_func.h"

GLOBAL_PTR(addr_dfu_img_base, ADDR_DFU_IMG_BASE);
GLOBAL_PTR(reg_base, 0x20a108000);

#define OFFSET_CTRL         0x2
#define OFFSET_STATUS       0x3
#define OFFSET_INT_STATUS   0x6
#define OFFSET_CMD_QUEUE    0x80

#define HX_OP_KEY       0x1
#define HX_OP_IV        0x2
#define HX_OP_DATA      0x5
#define HX_OP_SIGNAL    0x8

struct hx_cmd_key
{
    uint32_t id      : 8;
    uint32_t _pad3   : 8;
    uint32_t mode    : 2;
    uint32_t _pad2   : 2;
    uint32_t dir     : 1;
    uint32_t _pad1   : 1;
    uint32_t keysize : 2;
    uint32_t keytype : 3;
    uint32_t _pad0   : 1;
    uint32_t opcode  : 4;
};

struct hx_cmd_iv
{
    struct
    {
        uint32_t _pad0   : 28;
        uint32_t opcode  : 4;
    } cmd;
    uint32_t iv[4];
};

struct hx_cmd_data
{
    struct
    {
        uint32_t len     : 24;
        uint32_t _pad0   : 4;
        uint32_t opcode  : 4;
    } cmd;

    struct
    {
        uint32_t u_dst   : 8;
        uint32_t _pad0   : 8;
        uint32_t u_src   : 8;
    } u_addr;

    uint32_t src_addr;
    uint32_t dst_addr;
};

struct hx_cmd_signal
{
    uint32_t _pad0       : 26;
    uint32_t flag1       : 1;
    uint32_t flag0       : 1;
    uint32_t opcode      : 4;
};

PAYLOAD_SECTION
__attribute__ ((noinline))
static void hx_send_cmd(uint32_t *cmd, uint32_t len)
{
    int i;
    uint32_t *reg_base = GET_GLOBAL(reg_base);
    for(i = 0; i < len / sizeof(uint32_t); i++)
    {
        reg_base[OFFSET_CMD_QUEUE] = cmd[i];
        __asm__ volatile ("dmb oshst");
    }
}

PAYLOAD_SECTION
__attribute__ ((noinline))
static void fill_hx_cmd_key(struct hx_cmd_key *cmd,
                            struct hx_aes_ctx *ctx,
                            hx_dir_t dir, hx_mode_t mode)
{
    cmd->opcode = HX_OP_KEY;
    cmd->keytype = ctx->keytype;
    cmd->keysize = ctx->keysize;
    cmd->dir = dir;
    cmd->mode = mode;
    cmd->id = 0;
    cmd->_pad0 = 0;
    cmd->_pad1 = 0;
    cmd->_pad2 = 0;
}

PAYLOAD_SECTION
__attribute__ ((noinline))
static void fill_hx_cmd_iv(struct hx_cmd_iv *cmd, uint8_t *iv)
{
    cmd->cmd.opcode = HX_OP_IV;
    cmd->cmd._pad0 = 0;

    if(iv)
        dev_memcpy(cmd->iv, iv, sizeof(cmd->iv));
    else
        dev_memset(cmd->iv, 0, sizeof(cmd->iv));
}

PAYLOAD_SECTION
__attribute__ ((noinline))
static void fill_hx_cmd_data(struct hx_cmd_data *cmd,
                             unsigned int len,
                             uint8_t *src_phys, uint8_t *dst_phys)
{
    cmd->cmd.opcode = HX_OP_DATA;
    cmd->cmd.len = len;
    cmd->cmd._pad0 = 0;

    cmd->u_addr.u_src = (uint32_t) ((uint64_t) src_phys >> 32u);
    cmd->u_addr.u_dst = (uint32_t) ((uint64_t) dst_phys >> 32u);
    cmd->u_addr._pad0 = 0;

    cmd->src_addr = (uint32_t) ((uint64_t) src_phys & 0xFFFFFFFF);
    cmd->dst_addr = (uint32_t) ((uint64_t) dst_phys & 0xFFFFFFFF);
}

PAYLOAD_SECTION
__attribute__ ((noinline))
static void fill_hx_cmd_signal(struct hx_cmd_signal *cmd)
{
    cmd->opcode = HX_OP_SIGNAL;
    cmd->flag0 = 1;
    cmd->flag1 = 1;
    cmd->_pad0 = 0;
}

GLOBAL_PTR(is_seeded, 0x100001140)
GLOBAL_PTR(seed, 0x100002338)

PAYLOAD_SECTION
__attribute__ ((noinline))
static int hx_aes_generic(struct hx_aes_ctx *ctx, uint8_t *dst, uint8_t *src, unsigned int nbytes)
{
    int (*is_seeded)(void) = GET_GLOBAL(is_seeded);
    void (*seed)(void) = GET_GLOBAL(seed);

    int keysize;
    volatile uint32_t *reg_base = GET_GLOBAL(reg_base);

    struct hx_cmd_key cmd_key;
    struct hx_cmd_iv cmd_iv;
    struct hx_cmd_data cmd_data;
    struct hx_cmd_signal cmd_signal;

    fill_hx_cmd_key(&cmd_key, ctx, ctx->encdir, ctx->encmode);
    fill_hx_cmd_iv(&cmd_iv, ctx->iv);
    fill_hx_cmd_data(&cmd_data, nbytes, src, dst);
    fill_hx_cmd_signal(&cmd_signal);

    gpio_write(0x1905, 1);

    // turn on the clock
    clock_gate(0x3c, 1);

    // seed
    if(!is_seeded())
        seed();

    // clear interrupt and start hardware
    reg_base[OFFSET_INT_STATUS] = 0x20;
    __asm__ volatile ("dmb oshst");
    reg_base[OFFSET_CTRL] = 0x1;
    __asm__ volatile ("dmb oshst");

    hx_send_cmd((uint32_t *) &cmd_key, sizeof(struct hx_cmd_key));
    if(ctx->keytype == HX_KEY_USER)
    {
        keysize = 0;
        if(ctx->keysize == HX_KEYSIZE_128)
            keysize = 16;
        else if(ctx->keysize == HX_KEYSIZE_192)
            keysize = 24;
        else if(ctx->keysize == HX_KEYSIZE_256)
            keysize = 32;

        if(keysize != 0)
            hx_send_cmd((uint32_t *) ctx->key, keysize);
    }

    hx_send_cmd((uint32_t *) &cmd_iv, sizeof(struct hx_cmd_iv));
    hx_send_cmd((uint32_t *) &cmd_data, sizeof(struct hx_cmd_data));
    hx_send_cmd((uint32_t *) &cmd_signal, sizeof(struct hx_cmd_signal));

    // wait for interrupt
    while(reg_base[OFFSET_INT_STATUS] == 0)
        __asm__ volatile ("dmb oshld");

    reg_base[OFFSET_INT_STATUS] = 0x20;
    __asm__ volatile ("dmb oshst");
    reg_base[OFFSET_CTRL] = 0x2;
    __asm__ volatile ("dmb oshst");

    clock_gate(0x3c, 0);
    gpio_write(0x1905, 0);
    return 0;
}


PAYLOAD_SECTION
__attribute__ ((noinline))
void run()
{
    uint8_t result[16];
    struct hx_aes_ctx *ctx = GET_GLOBAL(addr_dfu_img_base);
    hx_aes_generic(ctx, &ctx->msg[0], result, 16);

    dev_memcpy(GET_GLOBAL(addr_dfu_img_base), result, 16);
}

void _start()
{
    run();
}