#include "checkm8.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <zconf.h>

#include "dev/types.h"
#include <dev/addr.h>
#include "util/experiments.h"
#include "util/host_crypto.h"

#ifdef CHECKM8_LOGGING

#include <stdarg.h>
#include <execinfo.h>


#endif

void checkm8_debug_indent(const char *format, ...)
{
#ifdef CHECKM8_LOGGING
    void *traces[100];
    int depth = backtrace(traces, 100) - 5;
    for(int i = 0; i < depth; i++)
    {
        printf("\t");
    }
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

void checkm8_debug_block(const char *format, ...)
{
#ifdef CHECKM8_LOGGING
    va_list args;

    va_start (args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

void record_bern_data(struct bern_data *data, int index)
{
    int j, b;
    FILE *outfile;
    char linebuf[256];

    sprintf(linebuf, "%i.dat", index);
    outfile = fopen(linebuf, "w+");
    if(outfile == NULL)
    {
        printf("failed to open data file\n");
        return;
    }

    sprintf(linebuf, "%lli %lli\n\n", data->count, data->ttotal);
    fputs(linebuf, outfile);

    for(j = 0; j < 16; j++)
    {
        for(b = 0; b < 256; b++)
        {
            sprintf(linebuf,
                    "%2d %3d %lli %lli %lli\n",
                    j, b, (long long) data->tnum[j][b], data->t[j][b], data->tsq[j][b]);
            fputs(linebuf, outfile);
        }
    }

    fclose(outfile);
}

void run_corr_exp(struct pwned_device *dev, char *fname)
{
    int i, j, iter = 0;
    char dat_fname[32];
    FILE *outfile;
    DEV_PTR_T addr_async_buf;

    struct aes_constants *c = get_constants();
    struct corr_data *data;

    unsigned char msg[16];
    unsigned char key[16];
    unsigned char key_sched[176];

    sprintf(dat_fname, "KEY");
    outfile = fopen(dat_fname, "w+");
    if(outfile == NULL)
    {
        printf("failed to open key file\n");
        return;
    }

    srand(time(NULL));
    for(i = 0; i < 16; i++)
    {
        msg[i] = 0;
        key[i] = random();
        fprintf(outfile, "%02x", key[i]);
    }

    fprintf(outfile, "\n");
    fflush(outfile);
    fclose(outfile);

    expand_key(key, key_sched, 11, c);

    addr_async_buf = setup_corr_exp(dev, key);
    printf("got async buf ptr %llx\n", addr_async_buf);
    if(addr_async_buf == DEV_PTR_NULL) return;

    while(1)
    {
        sprintf(dat_fname, "%s_%i.bin", fname, iter);
        outfile = fopen(dat_fname, "wb+");
        if(outfile == NULL)
        {
            printf("failed to open outfile\n");
            return;
        }

        for(j = 0; j < 375; j++)
        {
            data = get_corr_exp_data(dev, addr_async_buf);
            if(data->num_cutoff != 0)
                printf("more than 0 entries were cutoff\n");

            for(i = 0; i < N_CORR_ENTRIES; i++)
            {
                fwrite(msg, 1, sizeof(msg), outfile);
                fwrite("\x00", 1, 1, outfile);
                fwrite(&data->data[i], 1, 1, outfile);
                fwrite("\x00\x00", 1, 2, outfile);

                aes128_encrypt_ecb(msg, 16, key_sched, c);
            }

            fflush(outfile);
            for(i = 0; i < 16; i++)
            {
                if(msg[i] != data->msg[i])
                {
                    printf("aes error! message mismatch\n");
                    free(data);
                    return;
                }
            }

            free(data);
        }

        fclose(outfile);
        iter++;
        if(iter == 46) break;
    }
}

int main_bernstein()
{
    int i, count = 0;
    DEV_PTR_T async_buf;

    struct bern_data *data;
    struct pwned_device *dev = exploit_device();
    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    fix_heap(dev);

    async_buf = setup_bern_exp(dev);
    if(async_buf == DEV_PTR_NULL)
    {
        printf("failed to setup bernstein experiment\n");
        return -1;
    }

    while(1)
    {
        for(i = 0; i < 30; i++)
        {
            printf("sleeping %i / 30\n", i);
            sleep(60);
        }

        data = get_bern_exp_data(dev, async_buf);
        if(data == NULL)
        {
            printf("failed to get bernstein data\n");
            return -1;
        }

        record_bern_data(data, count);
        free(data);
        count++;
    }

    free_device(dev);
    return 0;
}

int main()
{
    return main_bernstein();
//    struct pwned_device *dev = exploit_device();
//    if(dev == NULL || dev->status == DEV_NORMAL)
//    {
//        printf("Failed to exploit device\n");
//        return -1;
//    }
//
//    demote_device(dev);
//    usb_task_exit(dev);
//
//    free_device(dev);
//    return 0;
}











