#include "bootrom_func.h"
#include "dev_util.h"
#include "dev_crypto.h"

PAYLOAD_SECTION
void entry_sync()
{

}

PAYLOAD_SECTION
void entry_async(uint64_t *base)
{
    int i;
    unsigned char key_sched[176];
    unsigned long long start, timing;

    unsigned char *key = (unsigned char *) base[0];
    struct aes_constants *c = (struct aes_constants *) base[1];

    struct corr_data *data = (struct corr_data *) base;
    event_new(&data->ev_cont, 1, 0);

    expand_key(key, key_sched, 11, c);
    for(i = 0; i < 16; i++)
        data->msg[i] = 0;

    while(1)
    {
        // reset data state
        data->num_cutoff = 0;
        for(i = 0; i < N_CORR_ENTRIES; i++)
        {
            data->data[i] = 0;
        }

        // collect new data
        i = 0;
        while(i < N_CORR_ENTRIES)
        {
            start = get_ticks();
            aes128_encrypt_ecb(data->msg, 16, key_sched, c);
            timing = get_ticks() - start;

            if(timing < 256)
                data->data[i++] = (unsigned char) timing;
            else
                data->num_cutoff++;
        }

        event_wait(&data->ev_cont);
    }

}