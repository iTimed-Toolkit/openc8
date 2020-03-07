#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "host_crypto.h"

int main(int argc, char *argv[])
{
    int b, i, j, k, l, num;

    unsigned char timing;
    unsigned char msg[16];
    unsigned char key[16];
    unsigned char key_sched[176];
    struct aes_constants *c;

    double t[16][256];
    double tsq[16][256];
    double tnum[16][256];
    double u[16][256];
    double udev[16][256];
    double taverage;
    unsigned long long count = 0, ttotal = 0;

    FILE *keyfile, *msgfile, *timefile, *outfile;
    char timing_name[256], msg_name[256], linebuf[256];

    if(argc != 2)
    {
        printf("usage: verify [data dir]\n");
        return -1;
    }

    sprintf(timing_name, "%s/KEY", argv[1]);
    keyfile = fopen(timing_name, "r");
    if(keyfile == NULL)
    {
        printf("failed to open key file\n");
        return -1;
    }

    for(i = 0; i < 16; i++)
    {
        fread(key_sched, 1, 2, keyfile);
        key_sched[2] = 0;
        key[i] = (unsigned char) strtol((char *) key_sched, NULL, 16);
    }

    fclose(keyfile);

    c = get_constants();
    expand_key(key, key_sched, 11, c);

    for(i = 0; i < 46; i++)
    {
        num = 0;
        sprintf(msg_name, "%s/msg_key00_%i.bin", argv[1], i);
        sprintf(timing_name, "%s/timing_key00_%i.bin", argv[1], i);

        msgfile = fopen(msg_name, "rb");
        if(msgfile == NULL)
        {
            printf("failed to open msgfile %s\n", msg_name);
            return -1;
        }

        timefile = fopen(timing_name, "rb");
        if(timefile == NULL)
        {
            printf("failed to open timing file %s\n", timing_name);
            return -1;
        }

        printf("file %i\n", i);

        for(j = 0; j < 375; j++)
        {
            fread(msg, 16, 1, msgfile);
            fread(&timing, 1, 1, timefile);

            printf("%i\t", num++);
            for(k = 0; k < 16; k++)
                printf("%02X", msg[k]);

            for(k = 0; k < 1024 * 256; k++)
            {
                for(l = 0; l < 16; l++)
                {
                    t[l][msg[l]] += timing;
                    tsq[l][msg[l]] += (timing * timing);
                    tnum[l][msg[l]] += 1;

                    count++;
                    ttotal += timing;
                }

                fread(&timing, 1, 1, timefile);
                aes128_encrypt_ecb(msg, 16, key_sched, c);
            }

            printf(" -> ");
            for(k = 0; k < 16; k++)
                printf("%02X", msg[k]);
            printf("\n");
        }

        fclose(msgfile);
        fclose(timefile);
    }

    taverage = ttotal / (double) count;

    for(j = 0; j < 16; j++)
    {
        for(b = 0; b < 256; b++)
        {
            u[j][b] = t[j][b] / tnum[j][b];
            udev[j][b] = tsq[j][b] / tnum[j][b];
            udev[j][b] -= u[j][b] * u[j][b];
            udev[j][b] = sqrt(udev[j][b]);
        }
    }

    sprintf(linebuf, "dat_%lli.dat", count / 16 / 100000);
    outfile = fopen(linebuf, "w+");
    if(outfile == NULL)
    {
        printf("failed to open data file\n");
        return -1;
    }

    for(j = 0; j < 16; j++)
    {
        for(b = 0; b < 256; b++)
        {
            sprintf(linebuf,
                    "%2d %3d %lli %f %f %f %f\n",
                    j, b, (long long) tnum[j][b],
                    u[j][b], udev[j][b],
                    u[j][b] - taverage, udev[j][b] / sqrt(tnum[j][b]));
            fputs(linebuf, outfile);
        }
    }

    fclose(outfile);
}