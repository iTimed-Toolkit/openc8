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

struct bern_exp_ptrs *
setup_bern_exp(struct pwned_device *dev, unsigned char key[16], unsigned int num_iter, unsigned int offset)
{
    struct dev_cmd_resp *resp;
    struct bern_exp_ptrs *res = malloc(sizeof(struct bern_exp_ptrs));

    unsigned char data[16] = {0};

    if(IS_CHECKM8_FAIL(open_device_session(dev)))
    {
        printf("failed to open device session\n");
        goto fail;
    }

    res->addr_constants = install_aes_data(dev, offset);
    if(res->addr_constants == DEV_PTR_NULL)
    {
        printf("failed to install aes constants\n");
        goto fail;
    }

    res->addr_data = install_data(dev, SRAM, data, 16);
    if(res->addr_data == DEV_PTR_NULL)
    {
        printf("failed to install aes data\n");
        goto fail;
    }

    res->addr_key = install_data(dev, SRAM, key, 16);
    if(res->addr_key == DEV_PTR_NULL)
    {
        printf("failed to install aes key\n");
        goto fail;
    }

    res->addr_results = get_address(dev, SRAM, sizeof(struct bern_data));
    if(res->addr_results == DEV_PTR_NULL)
    {
        printf("failed to create result array");
        goto fail;
    }

    resp = execute_gadget(dev, ADDR_EVENT_NEW, 0, 3,
                            res->addr_results + offsetof(struct bern_data, ev_data), 1, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to init data event\n");
        free_dev_cmd_resp(resp);
        goto fail;
    }

    free_dev_cmd_resp(resp);
    resp = execute_gadget(dev, ADDR_EVENT_NEW, 0, 3,
                          res->addr_results + offsetof(struct bern_data, ev_done), 1, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to init done event\n");
        free_dev_cmd_resp(resp);
        goto fail;
    }

    free_dev_cmd_resp(resp);

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_SYNC, SRAM)))
    {
        printf("failed to install sync payload\n");
        goto fail;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_AES_SW_BERN, SRAM)))
    {
        printf("failed to install aes payload\n");
        goto fail;
    }

    resp = execute_payload(dev, PAYLOAD_SYNC, 0, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to execute sync payload\n");
        free_dev_cmd_resp(resp);
        goto fail;
    }
    free_dev_cmd_resp(resp);

    if(IS_CHECKM8_FAIL(close_device_session(dev)))
    {
        printf("failed to close device session\n");
        goto fail;
    }

    return res;

fail:
    free(res);
    return NULL;
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
#else
    resp = execute_gadget(dev, ADDR_EVENT_WAIT, 0, 1,
                            async_buf + offsetof(struct bern_data, ev_done));
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to wait for data\n");
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