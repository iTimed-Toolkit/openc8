#include "bootrom_func.h"
#include "bootrom_type.h"
#include "cacheutil.h"

PAYLOAD_SECTION
void sub_bytes(unsigned char block[16], struct aes_constants *c)
{
    int i;
    unsigned char val;

    for(i = 0; i < 16; i++)
    {
        val = block[i];
        block[i] = c->sbox[val >> 4u][val & 0xfu];
    }
}

PAYLOAD_SECTION
void shift_rows(unsigned char block[16])
{
    unsigned char temp1, temp2;

    temp1 = block[0x1];
    block[0x1] = block[0x5];
    block[0x5] = block[0x9];
    block[0x9] = block[0xd];
    block[0xd] = temp1;

    temp1 = block[0x2];
    temp2 = block[0xe];
    block[0x2] = block[0xa];
    block[0xe] = block[0x6];
    block[0xa] = temp1;
    block[0x6] = temp2;

    temp1 = block[0x3];
    block[0x3] = block[0xf];
    block[0xf] = block[0xb];
    block[0xb] = block[0x7];
    block[0x7] = temp1;
}

PAYLOAD_SECTION
void mix_cols(unsigned char block[16], struct aes_constants *c)
{
    unsigned char r0, r1, r2, r3;
    int i;

    for(i = 0; i < 4; i++)
    {
        r0 = block[4 * i];
        r1 = block[4 * i + 1];
        r2 = block[4 * i + 2];
        r3 = block[4 * i + 3];

        // no reason for the "+ 0" here but it makes the code look more lined up :)
        block[4 * i + 0] = c->mul2[r0] ^ c->mul3[r1] ^ r2 ^ r3;
        block[4 * i + 1] = r0 ^ c->mul2[r1] ^ c->mul3[r2] ^ r3;
        block[4 * i + 2] = r0 ^ r1 ^ c->mul2[r2] ^ c->mul3[r3];
        block[4 * i + 3] = c->mul3[r0] ^ r1 ^ r2 ^ c->mul2[r3];
    }
}

PAYLOAD_SECTION
void add_key(unsigned char block[16], unsigned char key[16])
{
    int i;
    for(i = 0; i < 16; i++)
    {
        block[i] = block[i] ^ key[i];
    }
}

PAYLOAD_SECTION
void expand_key(unsigned char key[16], unsigned char key_sched[176], int n,
                struct aes_constants *c)
{
    int i, j, prev_key_base, key_base = 0;
    unsigned char val;
    for(i = 0; i < 16; i++)
    {
        key_sched[i] = key[i];
    }

    for(i = 1; i < n; i++)
    {
        prev_key_base = key_base;
        key_base = 16 * i;

        for(j = 0; j < 3; j++)
        {
            val = key_sched[prev_key_base + 13 + j];
            key_sched[key_base + j] = c->sbox[val >> 4u][val & 0xfu];
        }

        val = key_sched[prev_key_base + 12];
        key_sched[key_base + 3] = c->sbox[val >> 4u][val & 0xfu];

        key_sched[key_base] ^= c->rc_lookup[i - 1];

        for(j = 0; j < 4; j++)
        {
            key_sched[key_base + j] = key_sched[key_base + j] ^ key_sched[prev_key_base + j];
        }

        for(j = 4; j < 16; j++)
        {
            key_sched[key_base + j] = key_sched[key_base + j - 4] ^ key_sched[prev_key_base + j];
        }
    }
}

PAYLOAD_SECTION
void aes128_encrypt_ecb(unsigned char *msg, unsigned int msg_len,
                        unsigned char key_sched[176], struct aes_constants *c)
{
    unsigned int num_blocks = msg_len / 16;
    unsigned char *block;

    unsigned int i, j;
    for(i = 0; i < num_blocks; i++)
    {
        block = &msg[16 * i];
        add_key(block, key_sched);

        for(j = 0; j < 9; j++)
        {
            sub_bytes(block, c);
            shift_rows(block);
            mix_cols(block, c);
            add_key(block, &key_sched[16 * (j + 1)]);
        }

        sub_bytes(block, c);
        shift_rows(block);
        add_key(block, &key_sched[16 * (j + 1)]);
    }
}

PAYLOAD_SECTION
uint64_t entry_sync(unsigned char *msg, unsigned int msg_len, unsigned char key[16],
                    struct aes_constants *c)
{
    unsigned long long start = 0;
    unsigned char key_sched[176];
    expand_key(key, key_sched, 11, c);

    start = get_ticks();
    aes128_encrypt_ecb(msg, msg_len, key, c);
    return get_ticks() - start;
}

PAYLOAD_SECTION
void entry_async(uint64_t *base)
{
    int i, j, iter_count = 0;
    unsigned long long start = 0;
    struct event *usb_event = ADDR_USB_EVENT;

    unsigned char msg_old[16];
    unsigned char key_sched[176];
    double timing;

    // get initial params
    unsigned char *msg = (unsigned char *) base[0];
    unsigned int msg_len = (unsigned int) base[1];
    unsigned char *key = (unsigned char *) base[2];
    struct aes_constants *c = (struct aes_constants *) base[3];

    expand_key(key, key_sched, 11, c);

    // initialize events and buffers
    struct bern_data *data = (struct bern_data *) base;
    event_new(&data->ev_data, 1, 0);
    event_new(&data->ev_done, 1, 0);

    data->count = 0;
    for(i = 0; i < 16; i++)
    {
        for(j = 0; j < 256; j++)
        {
            data->t[i][j] = 0;
            data->tsq[i][j] = 0;
            data->tnum[i][j] = 0;
        }
    }

    while(1)
    {
        // randomly generate a new msg based on the old one
        for(i = 0; i < 16; i++)
            msg_old[i] = msg[i];

        // encrypt it and measure time
        start = get_ticks();
        aes128_encrypt_ecb(msg, msg_len, key_sched, c);
        timing = (double) (get_ticks() - start);

        // update counters
        for(i = 0; i < 16; i++)
        {
            data->t[i][msg_old[i]] += timing;
            data->tsq[i][msg_old[i]] += (timing * timing);
            data->tnum[i][msg_old[i]] += 1;

            data->count++;
            data->ttotal += timing;
        }

        // check if host has requested data
        iter_count++;
        if(iter_count % 100000 == 0)
        {
            if(event_try(&data->ev_data, 1))
                event_wait(&data->ev_done);
        }
    }
}