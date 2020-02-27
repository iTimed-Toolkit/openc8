#include "bootrom_func.h"
#include "bootrom_type.h"
#include "cacheutil.h"
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
void entry_async(uint64_t *base)
{
    int i, j, iter_count = 0;
    unsigned long long start = 0;

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