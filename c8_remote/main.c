#include "checkm8.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "dev/shared_types.h"
#include "util/experiments.h"
#include "tool/usb_helpers.h"

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

void record_bern_data(struct bern_data *data, unsigned char *key, int index, int prefix)
{
    int j, b;
    FILE *outfile;
    char linebuf[256];

    sprintf(linebuf, "%i", prefix);
    mkdir(linebuf, 0700);

    sprintf(linebuf, "%i/KEY-%i", prefix, index);
    outfile = fopen(linebuf, "w+");
    if(outfile == NULL)
    {
        printf("failed to open key file\n");
        return;
    }

    for(j = 0; j < 16; j++)
    {
        sprintf(linebuf, "%02X", key[j]);
        fputs(linebuf, outfile);
    }

    sprintf(linebuf, "\n");
    fputs(linebuf, outfile);

    fclose(outfile);

#ifdef BERNSTEIN_COLLECT1
    sprintf(linebuf, "%i/%i-1.dat", prefix, index);
    outfile = fopen(linebuf, "w+");
    if(outfile == NULL)
    {
        printf("failed to open data file\n");
        return;
    }

    sprintf(linebuf, "%lli %lli\n\n", data->count1, data->ttotal1);
    fputs(linebuf, outfile);

    for(j = 0; j < 128; j++)
    {
        for(b = 0; b < 2; b++)
        {
            sprintf(linebuf,
                    "%2d %3d %lli %lli %lli\n",
                    j, b, (long long) data->tnum1[j][b], data->t1[j][b], data->tsq1[j][b]);
            fputs(linebuf, outfile);
        }
    }

    fclose(outfile);
#endif

#ifdef BERNSTEIN_COLLECT2
    sprintf(linebuf, "%i/%i-2.dat", prefix, index);
    outfile = fopen(linebuf, "w+");
    if(outfile == NULL)
    {
        printf("failed to open data file\n");
        return;
    }

    sprintf(linebuf, "%lli %lli\n\n", data->count2, data->ttotal2);
    fputs(linebuf, outfile);

    for(j = 0; j < 64; j++)
    {
        for(b = 0; b < 4; b++)
        {
            sprintf(linebuf,
                    "%2d %3d %lli %lli %lli\n",
                    j, b, (long long) data->tnum2[j][b], data->t2[j][b], data->tsq2[j][b]);
            fputs(linebuf, outfile);
        }
    }

    fclose(outfile);
#endif

#ifdef BERNSTEIN_COLLECT4
    sprintf(linebuf, "%i/%i-4.dat", prefix, index);
    outfile = fopen(linebuf, "w+");
    if(outfile == NULL)
    {
        printf("failed to open data file\n");
        return;
    }

    sprintf(linebuf, "%lli %lli\n\n", data->count4, data->ttotal4);
    fputs(linebuf, outfile);

    for(j = 0; j < 32; j++)
    {
        for(b = 0; b < 16; b++)
        {
            sprintf(linebuf,
                    "%2d %3d %lli %lli %lli\n",
                    j, b, (long long) data->tnum4[j][b], data->t4[j][b], data->tsq4[j][b]);
            fputs(linebuf, outfile);
        }
    }

    fclose(outfile);
#endif

#ifdef BERNSTEIN_COLLECT8
    sprintf(linebuf, "%i/%i-8.dat", prefix, index);
    outfile = fopen(linebuf, "w+");
    if(outfile == NULL)
    {
        printf("failed to open data file\n");
        return;
    }

    sprintf(linebuf, "%lli %lli\n\n", data->count8, data->ttotal8);
    fputs(linebuf, outfile);

    for(j = 0; j < 16; j++)
    {
        for(b = 0; b < 256; b++)
        {
            sprintf(linebuf,
                    "%2d %3d %lli %lli %lli\n",
                    j, b, (long long) data->tnum8[j][b], data->t8[j][b], data->tsq8[j][b]);
            fputs(linebuf, outfile);
        }
    }

    fclose(outfile);
#endif
}

//int main_itimed(unsigned int num_iter, unsigned int offset)
//{
//    int i;
//    unsigned long long iter;
//    DEV_PTR_T argbuf;
//
//    struct bern_data *data;
//    struct bern_exp_ptrs *exp_ptrs;
//    struct dev_cmd_resp *resp;
//
//    unsigned char key[16];
//    memset(key, 0, 16);
//
//    struct pwned_device *dev = exploit_device();
//    if(dev == NULL || dev->status == DEV_NORMAL)
//    {
//        printf("Failed to exploit device\n");
//        return -1;
//    }
//
//    demote_device(dev);
//    fix_heap(dev);
//
//    exp_ptrs = setup_bern_exp(dev, key, num_iter, offset);
//    if(exp_ptrs == NULL)
//    {
//        printf("failed to set up bern experimnent\n");
//        return -1;
//    }
//
//    argbuf = get_address(dev, SRAM, 5 * sizeof(unsigned long long));
//    if(argbuf == DEV_PTR_NULL)
//    {
//        printf("failed to allocate arg buffer\n");
//        return -1;
//    }
//
//    resp = write_gadget(dev, argbuf, exp_ptrs, sizeof(struct bern_exp_ptrs));
//    if(IS_CHECKM8_FAIL(resp->ret))
//    {
//        printf("failed to write experiment pointers\n");
//        free_dev_cmd_resp(resp);
//        return -1;
//    }
//
//    free_dev_cmd_resp(resp);
//    double ratio = 1.0311772745930552;
//    for(i = 0; i < 32; i++)
//    {
//        for(int j = 0; j < 16; j++)
//            key[j] = random();
//
//        resp = write_gadget(dev, exp_ptrs->addr_key, key, 16);
//        if(IS_CHECKM8_FAIL(resp->ret))
//        {
//            printf("failed to write new key\n");
//            return -1;
//        }
//
//        resp = memset_gadget(dev, exp_ptrs->addr_results, 0, offsetof(struct bern_data, ev_data));
//        if(IS_CHECKM8_FAIL(resp->ret))
//        {
//            printf("failed to zero profile data\n");
//            return -1;
//        }
//
//        for(int curr_iter = 10000, last_iter = 0;
//            curr_iter <= num_iter;
//            last_iter = curr_iter, curr_iter *= ratio)
//        {
//            printf("starting profile %i - %i iterations\n", i, curr_iter);
//
//            iter = curr_iter - last_iter;
//            resp = write_gadget(dev, argbuf + sizeof(struct bern_exp_ptrs), &iter, sizeof(unsigned long long));
//            if(IS_CHECKM8_FAIL(resp->ret))
//            {
//                printf("failed to update number of iterations\n");
//                free_dev_cmd_resp(resp);
//                return -1;
//            }
//
//            free_dev_cmd_resp(resp);
//
//            if(async_payload_create(dev, PAYLOAD_AES_SW_BERN, argbuf) == DEV_PTR_NULL)
//            {
//                printf("failed to create a new task\n");
//                return -1;
//            }
//
//            if(IS_CHECKM8_FAIL(async_payload_run(dev, PAYLOAD_AES_SW_BERN)))
//            {
//                printf("failed to run async payload\n");
//                return -1;
//            }
//
//            data = get_bern_exp_data(dev, exp_ptrs->addr_results);
//            if(data == NULL)
//            {
//                printf("failed to get berstein data\n");
//                return -1;
//            }
//
//            async_payload_kill(dev, PAYLOAD_AES_SW_BERN, 0);
//            record_bern_data(data, key, i, curr_iter);
//            free(data);
//        }
//    }
//
//    free_device(dev);
//    return 0;
//}

int main_test_usb()
{
    struct pwned_device *dev = exploit_device();
    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    install_payload(dev, PAYLOAD_EXIT_USB_TASK);
    execute_payload(dev, PAYLOAD_EXIT_USB_TASK, NULL, 0, NULL, 0);

    close_device_session(dev);
    return 0;
}

int main()
{
    return main_test_usb();
}
