#include "bootrom_func.h"
#include "dev/shared_types.h"
#include "dev_cache.h"
#include "dev_crypto.h"


PAYLOAD_SECTION
void reset_data(struct bern_data *data)
{
    int i, j;

#ifdef BERNSTEIN_COLLECT1
    data->count1 = 0;
    data->ttotal1 = 0;

    for(i = 0; i < 128; i++)
    {
        for(j = 0; j < 2; j++)
        {
            data->t1[i][j] = 0;
            data->tsq1[i][j] = 0;
        }
    }
#endif

#ifdef BERNSTEIN_COLLECT2
    data->count2 = 0;
    data->ttotal2 = 0;

    for(i = 0; i < 64; i++)
    {
        for(j = 0; j < 4; j++)
        {
            data->t2[i][j] = 0;
            data->tsq2[i][j] = 0;
        }
    }
#endif

#ifdef BERNSTEIN_COLLECT4
    data->count4 = 0;
    data->ttotal4 = 0;

    for(i = 0; i < 32; i++)
    {
        for(j = 0; j < 16; j++)
        {
            data->t4[i][j] = 0;
            data->tsq4[i][j] = 0;
        }
    }
#endif

#ifdef BERNSTEIN_COLLECT8
    data->count8 = 0;
    data->ttotal8 = 0;

    for(i = 0; i < 16; i++)
    {
        for(j = 0; j < 256; j++)
        {
            data->t8[i][j] = 0;
            data->tsq8[i][j] = 0;
            data->tnum8[i][j] = 0;
        }
    }
#endif
}

PAYLOAD_SECTION
void update_data(struct bern_data *data, unsigned char *msg_old, unsigned long long timing)
{
    unsigned int i, j;
#ifdef BERNSTEIN_COLLECT1
    for(i = 0; i < 16; i++)
    {
        for(j = 0; j < 8; j++)
        {
            data->t1[8 * i + j][(msg_old[i] >> j) & 0b1u] += timing;
            data->tsq1[8 * i + j][(msg_old[i] >> j) & 0b1u] += (timing * timing);
            data->tnum1[8 * i + j][(msg_old[i] >> j) & 0b1u] += 1;

            data->count1++;
            data->ttotal1 += timing;
        }
    }
#endif

#ifdef BERNSTEIN_COLLECT2
    for(i = 0; i < 16; i++)
    {
        for(j = 0; j < 4; j++)
        {
            data->t2[4 * i + j][(msg_old[i] >> (2u * j)) & 0b11u] += timing;
            data->tsq2[4 * i + j][(msg_old[i] >> (2u * j)) & 0b11u] += (timing * timing);
            data->tnum2[4 * i + j][(msg_old[i] >> (2u * j)) & 0b11u] += 1;

            data->count2++;
            data->ttotal2 += timing;
        }
    }
#endif

#ifdef BERNSTEIN_COLLECT4
    for(i = 0; i < 16; i++)
    {
        for(j = 0; j < 2; j++)
        {
            data->t4[2 * i + j][(msg_old[i] >> (4u * j)) & 0b1111u] += timing;
            data->tsq4[2 * i + j][(msg_old[i] >> (4u * j)) & 0b1111u] += (timing * timing);
            data->tnum4[2 * i + j][(msg_old[i] >> (4u * j)) & 0b1111u] += 1;

            data->count4++;
            data->ttotal4 += timing;
        }
    }
#endif

#ifdef BERNSTEIN_COLLECT8
    for(i = 0; i < 16; i++)
    {
        data->t8[i][msg_old[i]] += timing;
        data->tsq8[i][msg_old[i]] += (timing * timing);
        data->tnum8[i][msg_old[i]] += 1;

        data->count8++;
        data->ttotal8 += timing;
    }
#endif
}

void _start(uint64_t *base)
{
    unsigned int i, j, iter_count = 0;
    unsigned long long start = 0;

    unsigned char msg_old[16];
    unsigned char key_sched[176];
    unsigned long long timing;

    // get initial params
    unsigned char *msg = (unsigned char *) base[0];
    unsigned char *key = (unsigned char *) base[1];
    struct bern_data *data = (struct bern_data *) base[2];

#ifndef BERNSTEIN_CONTINUOUS
    unsigned int num_iter = (unsigned int) base[3];
#endif

    expand_key(key, key_sched, 11);

    for(i = 0; i < 100000; i++)
        aes128_ttable_encrypt_ecb(msg, key_sched);

#ifdef BERNSTEIN_CONTINUOUS
    reset_data(data);
    while(1)
#else
    for(iter_count = 0; iter_count < num_iter; iter_count++)
#endif
    {
        // randomly generate a new msg based on the old one
        *((uint64_t *) &msg_old[0]) = *((uint64_t *) &msg[0]);
        *((uint64_t *) &msg_old[8]) = *((uint64_t *) &msg[8]);

        // encrypt it and measure time
        start = get_ticks();
        aes128_ttable_encrypt_ecb(msg, key_sched);
        timing = get_ticks() - start;

        // update counters
        update_data(data, msg_old, timing);

#ifdef BERNSTEIN_CONTINUOUS
        // check if host has requested data
        iter_count++;

#if defined(BERNSTEIN_WITH_USB)
        if(iter_count % 1000000 == 0)
        {
            if(event_try(&data->ev_data, 1))
            {
                event_wait(&data->ev_done);
                reset_data(data);
                iter_count = 0;
            }
        }
#else
        if(iter_count % 100000000 == 0)
        {
            __asm__ volatile ("b 0");
        }
#endif
#endif
    }

#ifndef BERNSTEIN_CONTINUOUS
//    event_notify(&data->ev_done);
    task_exit(0);
#endif
}