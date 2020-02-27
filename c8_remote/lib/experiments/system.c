#include "util/experiments.h"

#include <stdio.h>
#include "tool/command.h"

void usb_task_exit(struct pwned_device *dev)
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

    if(IS_CHECKM8_FAIL(install_payload(dev, PAYLOAD_EXIT_USB_TASK, SRAM)))
    {
        printf("failed to install sync payload\n");
        return;
    }

    resp = execute_payload(dev, PAYLOAD_SYNC, 0, 0);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to execute bootstrap\n");
        return;
    }
    free_dev_cmd_resp(resp);

    if(IS_CHECKM8_FAIL(uninstall_payload(dev, PAYLOAD_SYNC)))
    {
        printf("failed to uninstall sync payload\n");
        return;
    }

    resp = execute_payload(dev, PAYLOAD_EXIT_USB_TASK, 0,
                           1, get_payload_address(dev, PAYLOAD_EXIT_USB_TASK));
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        printf("failed to exit usb task\n");
        return;
    }

    if(IS_CHECKM8_FAIL(close_device_session(dev)))
    {
        printf("failed to close device session\n");
        return;
    }
}