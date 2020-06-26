#include "util/experiments.h"

#include <stdio.h>
#include "tool/payload.h"

void floppysleep(struct pwned_device *dev)
{
    struct dev_cmd_resp *resp;

    if(IS_CHECKM8_FAIL(open_device_session(dev)))
    {
        printf("failed to open device session\n");
        return;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_SYNC, SRAM)))
    {
        printf("failed to install sync payload\n");
        return;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_FLOPPYSLEEP, SRAM)))
    {
        printf("failed to install task sleep payload\n");
        return;
    }

    float init_a = -7.504355E-39f;
    DEV_PTR_T init_a_ptr = install_data(dev, SRAM, (unsigned char *) &init_a, sizeof(float));
    if(init_a_ptr == DEV_PTR_NULL)
    {
        printf("failed to write initial data\n");
        return;
    }

    resp = execute_payload(dev, PAYLOAD_SYNC, 0, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to execute bootstrap\n");
        return;
    }

    free_dev_cmd_resp(resp);

    resp = execute_payload(dev, PAYLOAD_FLOPPYSLEEP, 0, 1, init_a_ptr);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to execute flopsleep payload\n");
        return;
    }

    printf("retval is %08lli\n", resp->retval);
    free_dev_cmd_resp(resp);
    close_device_session(dev);
}

void floppysleep_async(struct pwned_device *dev)
{
    float init_a = -7.504355E-39f;
    DEV_PTR_T init_a_ptr, async_buf_ptr;
    struct dev_cmd_resp *resp;

    if(IS_CHECKM8_FAIL(open_device_session(dev)))
    {
        printf("failed to open device session\n");
        return;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_SYNC, SRAM)))
    {
        printf("failed to install sync payload\n");
        return;
    }

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_FLOPPYSLEEP, SRAM)))
    {
        printf("failed to install task sleep payload\n");
        return;
    }

    init_a_ptr = install_data(dev, SRAM, (unsigned char *) &init_a, sizeof(float));
    if(init_a_ptr == DEV_PTR_NULL)
    {
        printf("failed to write initial data\n");
        return;
    }

    resp = execute_payload(dev, PAYLOAD_SYNC, 0, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to execute bootstrap\n");
        return;
    }

    free_dev_cmd_resp(resp);

    async_buf_ptr = setup_payload_async_alloc(dev, PAYLOAD_FLOPPYSLEEP, 32, 1, init_a_ptr);
    run_payload_async(dev, PAYLOAD_FLOPPYSLEEP);
    close_device_session(dev);

    printf("async buf pointer is %llX\n", async_buf_ptr);

//    sleep(10);
//
//    open_device_session(dev);
//    resp = read_gadget(dev, async_buf_ptr, 8);
//    close_device_session(dev);
}