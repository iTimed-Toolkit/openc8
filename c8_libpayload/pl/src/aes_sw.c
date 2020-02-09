#include "util.h"
#include "brfunc_timing.h"

PAYLOAD_SECTION
void task_sleep(unsigned int usec)
{
    ((BOOTROM_FUNC) ADDR_TASK_SLEEP)(usec);
}

PAYLOAD_SECTION
void sub_bytes(unsigned char block[16], unsigned char sbox[16][16])
{
    int i;
    unsigned char val;

    for(i = 0; i < 16; i++)
    {
        val = block[i];
        block[i] = sbox[val >> 4u][val & 0xfu];
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
void mix_cols(unsigned char block[16],
              unsigned char mul2_lookup[256], unsigned char mul3_lookup[256])
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
        block[4 * i + 0] = mul2_lookup[r0] ^ mul3_lookup[r1] ^ r2 ^ r3;
        block[4 * i + 1] = r0 ^ mul2_lookup[r1] ^ mul3_lookup[r2] ^ r3;
        block[4 * i + 2] = r0 ^ r1 ^ mul2_lookup[r2] ^ mul3_lookup[r3];
        block[4 * i + 3] = mul3_lookup[r0] ^ r1 ^ r2 ^ mul2_lookup[r3];
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
                unsigned char sbox[16][16], unsigned char rc_lookup[11])
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
            key_sched[key_base + j] = sbox[val >> 4u][val & 0xfu];
        }

        val = key_sched[prev_key_base + 12];
        key_sched[key_base + 3] = sbox[val >> 4u][val & 0xfu];

        key_sched[key_base] ^= rc_lookup[i - 1];

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
void busy_sleep(int usec)
{
    unsigned long long halt = 0x1000004fc;
    unsigned long long timer_deadline_enter = 0x10000b874;
    unsigned long long now;

    __asm__ volatile ("mrs %0, cntpct_el0" : "=r" (now));
    ((BOOTROM_FUNC) timer_deadline_enter)(now + 24 * usec, ((BOOTROM_FUNC) 0x10000b924));
    ((BOOTROM_FUNC) halt)();
}

PAYLOAD_SECTION
void aes128_encrypt_ecb(unsigned char *msg, unsigned int msg_len, unsigned char key[16],
                        unsigned char sbox[16][16], unsigned char rc_lookup[11],
                        unsigned char mul2[256], unsigned char mul3[256])
{
    unsigned char key_sched[176];
    expand_key(key, key_sched, 11, sbox, rc_lookup);
    busy_sleep(10);

    unsigned int num_blocks = msg_len / 16;
    unsigned char *block;

    unsigned int i, j;
    for(i = 0; i < num_blocks; i++)
    {
        block = &msg[16 * i];
        add_key(block, key_sched);

        for(j = 0; j < 9; j++)
        {
            sub_bytes(block, sbox);
            shift_rows(block);
            mix_cols(block, mul2, mul3);
            add_key(block, &key_sched[16 * (j + 1)]);
        }

        sub_bytes(block, sbox);
        shift_rows(block);
        add_key(block, &key_sched[16 * (j + 1)]);
    }
}

TEXT_SECTION
unsigned long long _start(unsigned char *msg, unsigned int msg_len, unsigned char *key,
                    unsigned char sbox[16][16], unsigned char rc_lookup[11],
                    unsigned char mul2[256], unsigned char mul3[256])
{
    unsigned long long start = 0, end = 0;
    unsigned long long timer_deadline_enter = 0x10000b874;
    unsigned long long halt = 0x1000004fc;

    while(1)
    {
        __asm__ volatile ("mrs %0, cntpct_el0" : "=r" (start));
        aes128_encrypt_ecb(msg, msg_len, key, sbox, rc_lookup, mul2, mul3);
        __asm__ volatile ("mrs %0, cntpct_el0" : "=r" (end));

        if(2 * end - start - 64 > 0)
        {
            ((BOOTROM_FUNC) timer_deadline_enter)(2 * end - start - 64, ((BOOTROM_FUNC) 0x10000b924));
            ((BOOTROM_FUNC) halt)();
        }
    }

    return end - start;
}