#include "util/experiments.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "dev/addr.h"
#include "tool/command.h"
#include "util/host_crypto.h"

DEV_PTR_T install_aes_data(struct pwned_device *dev, unsigned int offset)
{
    int close;
    DEV_PTR_T res;
    struct aes_ttable_constants *constants = get_ttable_constants();

    if(is_device_session_open(dev)) close = 0;
    else
    {
        close = 1;
        if(IS_CHECKM8_FAIL(open_device_session(dev)))
        {
            printf("failed to open device session\n");
            free(constants);
            return DEV_PTR_NULL;
        }
    }

    res = install_data_offset(dev,
                              SRAM,
                              (unsigned char *) constants,
                              sizeof(struct aes_ttable_constants),
                              offset);
    if(res == DEV_PTR_NULL)
    {
        printf("failed to write AES constants\n");
        free(constants);
        return DEV_PTR_NULL;
    }

    if(close)
    {
        if(IS_CHECKM8_FAIL(close_device_session(dev)))
        {
            printf("failed to close device session\n");
            free(constants);
            return DEV_PTR_NULL;
        }
    }

    free(constants);
    return res;
}

DEV_PTR_T setup_bern_exp(struct pwned_device *dev, unsigned char key[16], unsigned int num_iter, unsigned int offset)
{
    DEV_PTR_T addr_data, addr_key, addr_async_buf, addr_constants;
    struct dev_cmd_resp *resp;
    FILE *keyfile;

    unsigned char data[16] = {0};

    keyfile = fopen("KEY", "w+");
    for(int i = 0; i < 16; i++)
        fprintf(keyfile, "%02X", key[i]);
    fclose(keyfile);

    if(IS_CHECKM8_FAIL(open_device_session(dev)))
    {
        printf("failed to open device session\n");
        return DEV_PTR_NULL;
    }

    addr_constants = install_aes_data(dev, offset);
    if(addr_constants == DEV_PTR_NULL)
    {
        printf("failed to install aes constants\n");
        return DEV_PTR_NULL;
    }

    addr_data = install_data(dev, SRAM, data, 16);
    if(addr_data == DEV_PTR_NULL)
    {
        printf("failed to install aes data\n");
        return DEV_PTR_NULL;
    }

    addr_key = install_data(dev, SRAM, key, 16);
    if(addr_key == DEV_PTR_NULL)
    {
        printf("failed to install aes key\n");
        return DEV_PTR_NULL;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_SYNC, SRAM)))
    {
        printf("failed to install sync payload\n");
        return DEV_PTR_NULL;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_AES_SW_BERN, SRAM)))
    {
        printf("failed to install aes payload\n");
        return DEV_PTR_NULL;
    }

    resp = execute_payload(dev, PAYLOAD_SYNC, 0, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to execute sync payload\n");
        free_dev_cmd_resp(resp);
        return DEV_PTR_NULL;
    }
    free_dev_cmd_resp(resp);

#ifdef BERNSTEIN_CONTINUOUS
    addr_async_buf = setup_payload_async(dev, PAYLOAD_AES_SW_BERN,
                                         sizeof(struct bern_data),
                                         3, addr_data, addr_key, addr_constants + offset);
#else
    addr_async_buf = setup_payload_async(dev, PAYLOAD_AES_SW_BERN,
                                         sizeof(struct bern_data),
                                         4, addr_data, addr_key, addr_constants + offset, num_iter);
#endif
    run_payload_async(dev, PAYLOAD_AES_SW_BERN);

    if(IS_CHECKM8_FAIL(close_device_session(dev)))
    {
        printf("failed to close device session\n");
        return DEV_PTR_NULL;
    }

    return addr_async_buf;
}

#ifdef BERNSTEIN_WITH_USB
struct bern_data *get_bern_exp_data(struct pwned_device *dev, DEV_PTR_T async_buf)
{
    struct dev_cmd_resp *resp;
    struct bern_data *res;

    if(IS_CHECKM8_FAIL(open_device_session(dev)))
    {
        printf("failed to open device session\n");
        return NULL;
    }

#ifdef BERNSTEIN_CONTINUOUS
    resp = execute_gadget(dev, ADDR_EVENT_NOTIFY, 0, 1,
                          async_buf + offsetof(struct bern_data, ev_data));
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to signal for data\n");
        free_dev_cmd_resp(resp);
        return NULL;
    }

    free_dev_cmd_resp(resp);
#endif
    resp = read_gadget(dev, async_buf, sizeof(struct bern_data));
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to get data from device\n");
        free_dev_cmd_resp(resp);
        return NULL;
    }

    res = (struct bern_data *) resp->data;
    free(resp);

#ifdef BERNSTEIN_CONTINUOUS
    resp = execute_gadget(dev, ADDR_EVENT_NOTIFY, 0, 1,
                          async_buf + offsetof(struct bern_data, ev_done));
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to signal data end\n");
        free(res);
        free_dev_cmd_resp(resp);
        return NULL;
    }

    free_dev_cmd_resp(resp);
#endif

    if(IS_CHECKM8_FAIL(close_device_session(dev)))
    {
        printf("failed to close device session\n");
        free(res);
        return NULL;
    }

    return res;
}
#endif