#include "payload.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "command.h"
#include "usb_helpers.h"

#include "bootrom_addr.h"
#include "libpayload.h"

struct payload
{
    PAYLOAD_T type;
    const unsigned char *data;
    int len;

    DEV_PTR_T install_base;
    struct payload *next;
    struct payload *prev;
};

struct payload *get_payload(PAYLOAD_T p)
{
    struct payload *res;
    const unsigned char *pl;
    int len;

    switch(p)
    {
        case PAYLOAD_AES_BUSY:
            pl = payload_aes_busy;
            len = PAYLOAD_AES_BUSY_SZ;
            break;

        case PAYLOAD_AES_SW:
            pl = payload_aes_sw;
            len = PAYLOAD_AES_SW_SZ;
            break;

        case PAYLOAD_EXIT_USB_TASK:
            pl = payload_exit_usb_task;
            len = PAYLOAD_EXIT_USB_TASK_SZ;
            break;

        case PAYLOAD_FLOPPYSLEEP:
            pl = payload_floppysleep;
            len = PAYLOAD_FLOPPYSLEEP_SZ;
            break;

        case PAYLOAD_SYNC:
            pl = payload_sync;
            len = PAYLOAD_SYNC_SZ;
            break;

        default:
            return NULL;
    }

    checkm8_debug_indent("get_payload(p = %i) -> %p\n", p, pl);
    res = malloc(sizeof(struct payload));
    if(res == NULL) return NULL;

    res->type = p;
    res->len = len;
    res->data = pl;
    res->install_base = DEV_PTR_NULL;
    res->next = NULL;
    res->prev = NULL;

    return res;
}

void free_payload(struct payload *p)
{
    free(p);
}

DEV_PTR_T get_address(struct pwned_device *dev, LOCATION_T l, int len)
{
    checkm8_debug_indent("get_address(dev = %p, loc = %i, len = %i)\n", dev, l, len);
    DEV_PTR_T retval;
    unsigned long long malloc_args[2] = {ADDR_DEV_MALLOC, (unsigned long long) len};

    struct dev_cmd_resp *resp = dev_exec(dev, 0, 2, malloc_args);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        free_dev_cmd_resp(resp);
        checkm8_debug_indent("\tfailed to malloc an address\n");
        return DEV_PTR_NULL;
    }

    retval = resp->retval;
    free_dev_cmd_resp(resp);

    checkm8_debug_indent("\tgot address %llX\n", retval);
    return retval;
}


struct payload *dev_retrieve_payload(struct pwned_device *dev, PAYLOAD_T p)
{
    struct payload *curr;
    for(curr = dev->installed; curr != NULL; curr = curr->next)
    {
        if(curr->type == p) return curr;
    }

    return NULL;
}

int dev_link_payload(struct pwned_device *dev, struct payload *pl)
{
    struct payload *curr;
    if(dev->installed == NULL)
    {
        dev->installed = pl;
        return CHECKM8_SUCCESS;
    }
    else
    {
        for(curr = dev->installed; curr->next != NULL; curr = curr->next);

        curr->next = pl;
        pl->prev = curr;
        return CHECKM8_SUCCESS;
    }
}

int *dev_unlink_payload(struct pwned_device *dev, struct payload *pl)
{
    if(dev->installed == pl)
    {
        dev->installed = pl->next;
        return CHECKM8_SUCCESS;
    }
    else
    {
        pl->prev->next = pl->next;
        if(pl->next != NULL)
            pl->next->prev = pl->prev;

        return CHECKM8_SUCCESS;
    }
}


int install_payload(struct pwned_device *dev, PAYLOAD_T p, LOCATION_T loc)
{
    checkm8_debug_indent("install_payload(dev = %p, p = %i, loc = %i)\n", dev, p, loc);

    struct dev_cmd_resp *resp = NULL;
    struct payload *pl = get_payload(p);
    DEV_PTR_T addr = get_address(dev, loc, pl->len);

    if(pl == NULL || addr == -1)
    {
        checkm8_debug_indent("\tinvalid args (either payload or address)\n");
        return CHECKM8_FAIL_INVARGS;
    }

    resp = dev_write_memory(dev, addr, (unsigned char *) pl->data, pl->len);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        free_dev_cmd_resp(resp);
        return CHECKM8_FAIL_XFER;
    }

    checkm8_debug_indent("\tdone copying and linking payload\n");
    pl->install_base = addr;
    dev_link_payload(dev, pl);

    free_dev_cmd_resp(resp);
    return CHECKM8_SUCCESS;
}

int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p)
{
    checkm8_debug_indent("uninstall payload(dev = %p, p = %i)\n", dev, p);
    unsigned long long free_args[2];
    struct dev_cmd_resp *resp;
    struct payload *pl = dev_retrieve_payload(dev, p);

    if(pl == NULL)
    {
        checkm8_debug_indent("\tinvalid args (payload)\n");
        return CHECKM8_FAIL_INVARGS;
    }

    free_args[0] = ADDR_DEV_FREE;
    free_args[1] = pl->install_base;

    resp = dev_exec(dev, 0, 2, free_args);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        free_dev_cmd_resp(resp);
        checkm8_debug_indent("\tfailed to free allocated payload memory\n");
        return CHECKM8_FAIL_XFER;
    }

    dev_unlink_payload(dev, pl);
    free_payload(pl);
    return CHECKM8_SUCCESS;
}

DEV_PTR_T get_payload_address(struct pwned_device *dev, PAYLOAD_T p)
{
    struct payload *pl = dev_retrieve_payload(dev, p);
    if(pl == NULL)
    {
        return DEV_PTR_NULL;
    }
    else
    {
        return pl->install_base;
    }
}


DEV_PTR_T install_data(struct pwned_device *dev, LOCATION_T loc, unsigned char *data, int len)
{
    checkm8_debug_indent("install_data(dev = %p, loc = %i, data = %p, len = %i)\n", dev, loc, data, len);
    struct dev_cmd_resp *resp;
    DEV_PTR_T addr = get_address(dev, loc, len);

    if(addr == -1)
    {
        checkm8_debug_indent("\tfailed to get an address\n");
        return DEV_PTR_NULL;
    }

    checkm8_debug_indent("\twriting data to address %X\n", addr);
    resp = dev_write_memory(dev, addr, data, len);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        checkm8_debug_indent("\tfailed to write data\n");
        return -1;
    }

    free_dev_cmd_resp(resp);
    return addr;
}

int uninstall_data(struct pwned_device *dev, DEV_PTR_T addr)
{
    checkm8_debug_indent("uninstall_data(dev = %p, addr = %X)\n", dev, addr);
    struct dev_cmd_resp *resp;
    unsigned long long free_args[2] = {ADDR_DEV_FREE, addr};

    resp = dev_exec(dev, 0, 2, free_args);
    if(IS_CHECKM8_FAIL(resp->ret))
    {
        checkm8_debug_indent("failed to free memory at %x\n", addr);
        return CHECKM8_FAIL_XFER;
    }

    return CHECKM8_SUCCESS;
}

struct dev_cmd_resp *execute_payload(struct pwned_device *dev, PAYLOAD_T p, int response_len, int nargs, ...)
{
    checkm8_debug_indent("execute_payload(dev = %p, p = %i, response_len = %i, nargs = %i, ...)\n",
                         dev, p, response_len, nargs);
    int i;
    struct dev_cmd_resp *resp;
    struct payload *pl;

    if((pl = dev_retrieve_payload(dev, p)) == NULL)
    {
        checkm8_debug_indent("\tpayload is not installed\n");
        resp = calloc(1, sizeof(struct dev_cmd_resp));
        resp->ret = CHECKM8_FAIL_NOINST;
        return resp;
    }

    unsigned long long args[nargs + 1];
    args[0] = pl->install_base;
    checkm8_debug_indent("\tinstall base is 0x%lX\n", args[0]);

    va_list arg_list;
    va_start(arg_list, nargs);
    for(i = 0; i < nargs; i++)
    {
        args[i + 1] = va_arg(arg_list, unsigned long long);
        checkm8_debug_indent("\textracted arg %lx\n", args[i + 1]);
    }
    va_end(arg_list);

    return dev_exec(dev, response_len, nargs + 1, args);
}

unsigned long long execute_payload_async(struct pwned_device *dev, PAYLOAD_T p, int bufsize, int nargs, ...)
{
    checkm8_debug_indent("execute_payload_async(dev = %p, p = %i, bufsize = %i, nargs = %i, ...)\n",
                         dev, p, bufsize, bufsize, nargs);
    int i;
    struct dev_cmd_resp *resp;
    struct payload *pl;

    if((pl = dev_retrieve_payload(dev, p)) == NULL)
    {
        checkm8_debug_indent("\tpayload is not installed\n");
        return DEV_PTR_NULL;
    }

    if(bufsize < nargs * sizeof(unsigned long long))
    {
        checkm8_debug_indent("\texpanding buffer to fit (at least) provided arguments\n");
        bufsize = nargs * sizeof(unsigned long long);
    }

    
}

struct dev_cmd_resp *read_gadget(struct pwned_device *dev, DEV_PTR_T addr, int len)
{
    checkm8_debug_indent("read_gadget(dev = %p, addr = %lx, len = %i)\n", dev, addr, len);
    return dev_read_memory(dev, addr, len);
}

struct dev_cmd_resp *write_gadget(struct pwned_device *dev, DEV_PTR_T addr, unsigned char *data, int len)
{
    checkm8_debug_indent("write_gadget(dev = %p, addr = %lx, data = %p, len = %i)\n", dev, addr, data, len);
    return dev_write_memory(dev, addr, data, len);
}

struct dev_cmd_resp *execute_gadget(struct pwned_device *dev, DEV_PTR_T addr, int response_len, int nargs, ...)
{
    checkm8_debug_indent("execute_gadget(dev = %p, addr = %lx, nargs = %i)\n", dev, addr, nargs);
    int i;

    unsigned long long args[nargs + 1];
    args[0] = addr;

    va_list arg_list;
    va_start(arg_list, nargs);
    for(i = 0; i < nargs; i++)
    {
        args[i + 1] = va_arg(arg_list, unsigned long long);
        checkm8_debug_indent("\textracted arg %lx\n", args[i + 1]);
    }
    va_end(arg_list);

    return dev_exec(dev, response_len, nargs + 1, args);
}