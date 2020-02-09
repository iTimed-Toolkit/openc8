#include "checkm8.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <float.h>

#include "command.h"
#include "payload.h"
#include "usb_helpers.h"

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

int floppysleep(struct pwned_device *dev)
{
    struct dev_cmd_resp *resp;

    if(IS_CHECKM8_FAIL(open_device_session(dev)))
    {
        printf("failed to open device session\n");
        return -1;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_SYNC, SRAM)))
    {
        printf("failed to install sync payload\n");
        return -1;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_FLOPPYSLEEP, SRAM)))
    {
        printf("failed to install task sleep payload\n");
        return -1;
    }

    float init_a = -7.504355E-39f;
    resp = write_gadget(dev, 0x180154000, (unsigned char *) &init_a, sizeof(float));
    free_dev_cmd_resp(resp);

    resp = execute_payload(dev, PAYLOAD_SYNC, 0, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to execute bootstrap\n");
        return -1;
    }

    free_dev_cmd_resp(resp);

    while(1)
    {
        resp = execute_payload(dev, PAYLOAD_FLOPPYSLEEP, 0, 1, 0x180154000);
        if(IS_CHECKM8_FAIL(resp->ret))
        {
            printf("failed to execute flopsleep payload\n");
            return -1;
        }

        printf("retval is %08lli\n", resp->retval);
        free_dev_cmd_resp(resp);

        usleep(2000000);
    }

    close_device_session(dev);
    free_device(dev);
}

void aes_sw(struct pwned_device *dev)
{
    int i = 0;
    struct dev_cmd_resp *resp;
    unsigned long long addr_sbox, addr_rc, addr_mul2, addr_mul3, addr_key, addr_data;

    unsigned char key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
                             0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    unsigned char data[16] = {0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef,
                              0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef};

    unsigned char sbox[256] =
            {
                    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
                    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
                    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
                    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
                    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
                    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
                    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
                    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
                    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
                    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
                    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
                    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
                    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
                    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
                    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
                    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
            };

    unsigned char rc_lookup[11] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c};

    unsigned char mul2_lookup[256] =
            {
                    0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e,
                    0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e,
                    0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,
                    0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e,
                    0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e,
                    0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe,
                    0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde,
                    0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe,
                    0x1b, 0x19, 0x1f, 0x1d, 0x13, 0x11, 0x17, 0x15, 0x0b, 0x09, 0x0f, 0x0d, 0x03, 0x01, 0x07, 0x05,
                    0x3b, 0x39, 0x3f, 0x3d, 0x33, 0x31, 0x37, 0x35, 0x2b, 0x29, 0x2f, 0x2d, 0x23, 0x21, 0x27, 0x25,
                    0x5b, 0x59, 0x5f, 0x5d, 0x53, 0x51, 0x57, 0x55, 0x4b, 0x49, 0x4f, 0x4d, 0x43, 0x41, 0x47, 0x45,
                    0x7b, 0x79, 0x7f, 0x7d, 0x73, 0x71, 0x77, 0x75, 0x6b, 0x69, 0x6f, 0x6d, 0x63, 0x61, 0x67, 0x65,
                    0x9b, 0x99, 0x9f, 0x9d, 0x93, 0x91, 0x97, 0x95, 0x8b, 0x89, 0x8f, 0x8d, 0x83, 0x81, 0x87, 0x85,
                    0xbb, 0xb9, 0xbf, 0xbd, 0xb3, 0xb1, 0xb7, 0xb5, 0xab, 0xa9, 0xaf, 0xad, 0xa3, 0xa1, 0xa7, 0xa5,
                    0xdb, 0xd9, 0xdf, 0xdd, 0xd3, 0xd1, 0xd7, 0xd5, 0xcb, 0xc9, 0xcf, 0xcd, 0xc3, 0xc1, 0xc7, 0xc5,
                    0xfb, 0xf9, 0xff, 0xfd, 0xf3, 0xf1, 0xf7, 0xf5, 0xeb, 0xe9, 0xef, 0xed, 0xe3, 0xe1, 0xe7, 0xe5
            };

    unsigned char mul3_lookup[256] =
            {
                    0x00, 0x03, 0x06, 0x05, 0x0c, 0x0f, 0x0a, 0x09, 0x18, 0x1b, 0x1e, 0x1d, 0x14, 0x17, 0x12, 0x11,
                    0x30, 0x33, 0x36, 0x35, 0x3c, 0x3f, 0x3a, 0x39, 0x28, 0x2b, 0x2e, 0x2d, 0x24, 0x27, 0x22, 0x21,
                    0x60, 0x63, 0x66, 0x65, 0x6c, 0x6f, 0x6a, 0x69, 0x78, 0x7b, 0x7e, 0x7d, 0x74, 0x77, 0x72, 0x71,
                    0x50, 0x53, 0x56, 0x55, 0x5c, 0x5f, 0x5a, 0x59, 0x48, 0x4b, 0x4e, 0x4d, 0x44, 0x47, 0x42, 0x41,
                    0xc0, 0xc3, 0xc6, 0xc5, 0xcc, 0xcf, 0xca, 0xc9, 0xd8, 0xdb, 0xde, 0xdd, 0xd4, 0xd7, 0xd2, 0xd1,
                    0xf0, 0xf3, 0xf6, 0xf5, 0xfc, 0xff, 0xfa, 0xf9, 0xe8, 0xeb, 0xee, 0xed, 0xe4, 0xe7, 0xe2, 0xe1,
                    0xa0, 0xa3, 0xa6, 0xa5, 0xac, 0xaf, 0xaa, 0xa9, 0xb8, 0xbb, 0xbe, 0xbd, 0xb4, 0xb7, 0xb2, 0xb1,
                    0x90, 0x93, 0x96, 0x95, 0x9c, 0x9f, 0x9a, 0x99, 0x88, 0x8b, 0x8e, 0x8d, 0x84, 0x87, 0x82, 0x81,
                    0x9b, 0x98, 0x9d, 0x9e, 0x97, 0x94, 0x91, 0x92, 0x83, 0x80, 0x85, 0x86, 0x8f, 0x8c, 0x89, 0x8a,
                    0xab, 0xa8, 0xad, 0xae, 0xa7, 0xa4, 0xa1, 0xa2, 0xb3, 0xb0, 0xb5, 0xb6, 0xbf, 0xbc, 0xb9, 0xba,
                    0xfb, 0xf8, 0xfd, 0xfe, 0xf7, 0xf4, 0xf1, 0xf2, 0xe3, 0xe0, 0xe5, 0xe6, 0xef, 0xec, 0xe9, 0xea,
                    0xcb, 0xc8, 0xcd, 0xce, 0xc7, 0xc4, 0xc1, 0xc2, 0xd3, 0xd0, 0xd5, 0xd6, 0xdf, 0xdc, 0xd9, 0xda,
                    0x5b, 0x58, 0x5d, 0x5e, 0x57, 0x54, 0x51, 0x52, 0x43, 0x40, 0x45, 0x46, 0x4f, 0x4c, 0x49, 0x4a,
                    0x6b, 0x68, 0x6d, 0x6e, 0x67, 0x64, 0x61, 0x62, 0x73, 0x70, 0x75, 0x76, 0x7f, 0x7c, 0x79, 0x7a,
                    0x3b, 0x38, 0x3d, 0x3e, 0x37, 0x34, 0x31, 0x32, 0x23, 0x20, 0x25, 0x26, 0x2f, 0x2c, 0x29, 0x2a,
                    0x0b, 0x08, 0x0d, 0x0e, 0x07, 0x04, 0x01, 0x02, 0x13, 0x10, 0x15, 0x16, 0x1f, 0x1c, 0x19, 0x1a
            };

    if(IS_CHECKM8_FAIL(open_device_session(dev)))
    {
        printf("failed to open device session\n");
        return;
    }

    addr_sbox = install_data(dev, SRAM, sbox, 256);
    if(addr_sbox == -1)
    {
        printf("failed to write sbox\n");
        return;
    }

    addr_rc = install_data(dev, SRAM, rc_lookup, 11);
    if(addr_rc == -1)
    {
        printf("failed to write rc lookup\n");
        return;
    }

    addr_mul2 = install_data(dev, SRAM, mul2_lookup, 256);
    if(addr_mul2 == -1)
    {
        printf("failed to write mul2 lookup\n");
        return;
    }

    addr_mul3 = install_data(dev, SRAM, mul3_lookup, 256);
    if(addr_mul3 == -1)
    {
        printf("failed to write mul3 lookup\n");
        return;
    }

    addr_key = install_data(dev, SRAM, key, 16);
    if(addr_key == -1)
    {
        printf("failed to write key\n");
        return;
    }

    addr_data = install_data(dev, SRAM, data, 16);
    if(addr_data == -1)
    {
        printf("failed to write data\n");
        return;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_SYNC, SRAM)))
    {
        printf("failed to install sync payload\n");
        return;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_AES_SW, SRAM)))
    {
        printf("failed to install task sleep payload\n");
        return;
    }

    for(i = 0; i < 100; i++)
    {
        resp = execute_payload(dev, PAYLOAD_AES_SW, 0, 7,
                               addr_data, 16, addr_key,
                               addr_sbox, addr_rc, addr_mul2, addr_mul3);
        if(IS_CHECKM8_FAIL(resp->ret))
        {
            printf("failed to execute sw AES payload\n");
            return;
        }

        printf("%i) op took %llu\n", i++, resp->retval);

        free_dev_cmd_resp(resp);
        resp = read_gadget(dev, addr_data, 16);
        if(IS_CHECKM8_FAIL(resp->ret))
        {
            printf("failed to read encrypted data from memory\n");
        }

        printf(" -> ");
        for(int j = 0; j < 16; j++)
        {
            printf("%02x", resp->data[j]);
        }
        printf("\n");

        free_dev_cmd_resp(resp);
        usleep(1000000);
    }

    uninstall_payload(dev, PAYLOAD_AES_SW);
    uninstall_payload(dev, PAYLOAD_SYNC);

    uninstall_data(dev, addr_data);
    uninstall_data(dev, addr_key);
    uninstall_data(dev, addr_mul3);
    uninstall_data(dev, addr_mul2);
    uninstall_data(dev, addr_rc);
    uninstall_data(dev, addr_sbox);

    close_device_session(dev);
}

int main()
{
    struct dev_cmd_resp *resp;
    struct pwned_device *dev = exploit_device();
    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    demote_device(dev);
    aes_sw(dev);

//    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_SYNC, SRAM)))
//    {
//        printf("failed to install sync payload\n");
//        return -1;
//    }
//
//    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_TASK_SLEEP_TEST, SRAM)))
//    {
//        printf("failed to install exit usb task payload\n");
//        return -1;
//    }
//
//    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_FLOPPYSLEEP, SRAM)))
//    {
//        printf("failed to install floppysleep\n");
//        return -1;
//    }
//
//    float init_a = -7.504355E-39f;
//    resp = write_gadget(dev, 0x180154000, (unsigned char *) &init_a, sizeof(float));
//    free_dev_cmd_resp(resp);
//
//    resp = execute_payload(dev, PAYLOAD_SYNC, 0, 0);
//    if(IS_CHECKM8_FAIL(resp->ret))
//    {
//        printf("failed to execute bootstrap\n");
//        return -1;
//    }
//    free_dev_cmd_resp(resp);
//
//    resp = execute_payload(dev, PAYLOAD_TASK_SLEEP_TEST, 0, 2, 0x180152000, 0x180154000);
//    if(IS_CHECKM8_FAIL(resp->ret))
//    {
//        printf("failed to exit usb task\n");
//        return -1;
//    }
//    free_dev_cmd_resp(resp);
//
//    close_device_session(dev);
    free_device(dev);
}











