#include "bootrom_func.h"
#include "dev/types.h"
#include "dev_cache.h"
#include "dev_crypto.h"

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
void reset_data(struct bern_data *data)
{
    int i, j;

    data->count = 0;
    data->ttotal = 0;

    for(i = 0; i < 16; i++)
    {
        for(j = 0; j < 256; j++)
        {
            data->t[i][j] = 0;
            data->tsq[i][j] = 0;
            data->tnum[i][j] = 0;
        }
    }
}

PAYLOAD_SECTION
void entry_async(uint64_t *base)
{
    int i, j, iter_count = 0;
    unsigned long long start = 0;

    unsigned char msg_old[16];
    unsigned char key_sched[176];
    unsigned long long timing;

    // get initial params
    unsigned char *msg = (unsigned char *) base[0];
    unsigned int msg_len = (unsigned int) base[1];
    unsigned char *key = (unsigned char *) base[2];
    struct aes_constants *c = (struct aes_constants *) base[3];

#ifndef BERNSTEIN_CONTINUOUS
    unsigned int num_iter = (unsigned int) base[4];
#endif

    expand_key(key, key_sched, 11, c);

    // initialize events and buffers
    struct bern_data *data = (struct bern_data *) base;
#ifdef BERNSTEIN_WITH_USB
    event_new(&data->ev_data, 1, 0);
    event_new(&data->ev_done, 1, 0);
#elif defined(BERNSTEIN_CONTINUOUS)
    // initial hook
    __asm__ volatile ("b 0");
#endif

    reset_data(data);
#ifdef BERNSTEIN_CONTINUOUS
    while(1)
#else
    for(iter_count = 0; iter_count < num_iter; iter_count++)
#endif
    {
        // randomly generate a new msg based on the old one
        for(i = 0; i < 16; i++)
            msg_old[i] = msg[i];

        // encrypt it and measure time
        start = get_ticks();
        aes128_encrypt_ecb(msg, msg_len, key_sched, c);
        timing = get_ticks() - start;

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
#ifdef BERNSTEIN_CONTINUOUS
        iter_count++;

#if defined(BERNSTEIN_WITH_USB)
        if(iter_count % 100000 == 0)
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
    task_exit(0);
#endif
}