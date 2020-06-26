#include "dev/types.h"

#ifdef DEV_CRYPTO
#include "dev_util.h"
#else

#include <stdint.h>

#endif

#ifdef DEV_CRYPTO
PAYLOAD_SECTION
#endif

void sub_bytes(unsigned char block[16], struct aes_sbox_constants *c)
{
    int i;
    uint8_t val;

    for(i = 0; i < 16; i++)
    {
        val = block[i];
        block[i] = c->sbox[val];
    }
}

#ifdef DEV_CRYPTO
PAYLOAD_SECTION
#endif

void shift_rows(unsigned char block[16])
{
    uint8_t temp1, temp2;

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

#ifdef DEV_CRYPTO
PAYLOAD_SECTION
#endif

void mix_cols(unsigned char block[16], struct aes_sbox_constants *c)
{
    uint8_t r0, r1, r2, r3;
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

#ifdef DEV_CRYPTO
PAYLOAD_SECTION
#endif

void add_key(unsigned char block[16], unsigned char key[16])
{
    int i;
    for(i = 0; i < 16; i++)
    {
        block[i] = block[i] ^ key[i];
    }
}

static inline void expand_key(unsigned char key[16], unsigned char key_sched[176], int n,
                              unsigned char *sbox, unsigned char *rc_lookup)
{
    int i, j, prev_key_base, key_base = 0;
    uint8_t val;
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
            key_sched[key_base + j] = sbox[val];
        }

        val = key_sched[prev_key_base + 12];
        key_sched[key_base + 3] = sbox[val];

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

#ifdef DEV_CRYPTO
PAYLOAD_SECTION
#endif

void expand_key_sbox(unsigned char key[16], unsigned char key_sched[176], int n, struct aes_sbox_constants *c)
{
    expand_key(key, key_sched, n, c->sbox, c->rc_lookup);
}

#ifdef DEV_CRYPTO
PAYLOAD_SECTION
#endif

void expand_key_ttable(unsigned char key[16], unsigned char key_sched[176], int n, struct aes_ttable_constants *c)
{
    expand_key(key, key_sched, n, c->sbox, c->rc_lookup);
}

#ifdef DEV_CRYPTO
PAYLOAD_SECTION
#endif

void aes128_ttable_encrypt_ecb(unsigned char *msg,
                               unsigned char key_sched[176], struct aes_ttable_constants *c)
{
    int i;

    uint32_t *key_sched_int = (uint32_t *) key_sched;
    uint32_t *msg_int = (uint32_t *) msg;
    uint32_t t0, t1, t2, t3, u0, u1, u2, u3;

    t0 = msg_int[0] ^ key_sched_int[0];
    t1 = msg_int[1] ^ key_sched_int[1];
    t2 = msg_int[2] ^ key_sched_int[2];
    t3 = msg_int[3] ^ key_sched_int[3];

    // 9 rounds
    for(i = 1; i < 10; ++i)
    {
        u0 = c->t0[(unsigned char) t0] ^
             c->t1[(unsigned char) (t1 >> 8u)] ^
             c->t2[(unsigned char) (t2 >> 16u)] ^
             c->t3[t3 >> 24u];

        u1 = c->t0[(unsigned char) t1] ^
             c->t1[(unsigned char) (t2 >> 8u)] ^
             c->t2[(unsigned char) (t3 >> 16u)] ^
             c->t3[t0 >> 24u];

        u2 = c->t0[(unsigned char) t2] ^
             c->t1[(unsigned char) (t3 >> 8u)] ^
             c->t2[(unsigned char) (t0 >> 16u)] ^
             c->t3[t1 >> 24u];

        u3 = c->t0[(unsigned char) t3] ^
             c->t1[(unsigned char) (t0 >> 8u)] ^
             c->t2[(unsigned char) (t1 >> 16u)] ^
             c->t3[t2 >> 24u];

        t0 = u0 ^ key_sched_int[4 * i];
        t1 = u1 ^ key_sched_int[4 * i + 1];
        t2 = u2 ^ key_sched_int[4 * i + 2];
        t3 = u3 ^ key_sched_int[4 * i + 3];
    }

    msg_int[0] = ((unsigned int) c->sbox[(unsigned char) (t0 >> 0u)] << 0u |
                  (unsigned int) c->sbox[(unsigned char) (t1 >> 8u)] << 8u |
                  (unsigned int) c->sbox[(unsigned char) (t2 >> 16u)] << 16u |
                  (unsigned int) c->sbox[(unsigned char) (t3 >> 24u)] << 24u) ^
                 key_sched_int[4 * i];

    msg_int[1] = ((unsigned int) c->sbox[(unsigned char) (t1 >> 0u)] << 0u |
                  (unsigned int) c->sbox[(unsigned char) (t2 >> 8u)] << 8u |
                  (unsigned int) c->sbox[(unsigned char) (t3 >> 16u)] << 16u |
                  (unsigned int) c->sbox[(unsigned char) (t0 >> 24u)] << 24u) ^
                 key_sched_int[4 * i + 1];

    msg_int[2] = ((unsigned int) c->sbox[(unsigned char) (t2 >> 0u)] << 0u |
                  (unsigned int) c->sbox[(unsigned char) (t3 >> 8u)] << 8u |
                  (unsigned int) c->sbox[(unsigned char) (t0 >> 16u)] << 16u |
                  (unsigned int) c->sbox[(unsigned char) (t1 >> 24u)] << 24u) ^
                 key_sched_int[4 * i + 2];

    msg_int[3] = ((unsigned int) c->sbox[(unsigned char) (t3 >> 0u)] << 0u |
                  (unsigned int) c->sbox[(unsigned char) (t0 >> 8u)] << 8u |
                  (unsigned int) c->sbox[(unsigned char) (t1 >> 16u)] << 16u |
                  (unsigned int) c->sbox[(unsigned char) (t2 >> 24u)] << 24u) ^
                 key_sched_int[4 * i + 3];
}

#ifdef DEV_CRYPTO
PAYLOAD_SECTION
#endif

void aes128_sbox_encrypt_ecb(unsigned char *msg,
                             unsigned char key_sched[176], struct aes_sbox_constants *c)
{
    int j;

    add_key(msg, key_sched);

    for(j = 0; j < 9; j++)
    {
        sub_bytes(msg, c);
        shift_rows(msg);
        mix_cols(msg, c);
        add_key(msg, &key_sched[16 * (j + 1)]);
    }

    sub_bytes(msg, c);
    shift_rows(msg);
    add_key(msg, &key_sched[16 * (j + 1)]);
}