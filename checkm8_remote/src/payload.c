#include "payload.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "command.h"
#include "libusb_helpers.h"


struct payload
{
    PAYLOAD_T type;
    unsigned char *data;
    long len;

    long install_base;
    struct payload *next;
    struct payload *prev;
};

struct payload *get_payload(PAYLOAD_T p)
{
    FILE *payload_file;
    struct payload *res;
    char *path;

    switch(p)
    {
        case PAYLOAD_AES:
            path = PAYLOAD_AES_BIN;
            break;

        case PAYLOAD_SYSREG:
            path = PAYLOAD_SYSREG_BIN;
            break;

        default:
            return NULL;
    }

    res = malloc(sizeof(struct payload));
    if(res == NULL) return NULL;

    if((payload_file = fopen(path, "rb")) == NULL)
    {
        free(res);
        return NULL;
    }

    fseek(payload_file, 0, SEEK_END);
    res->type = p;
    res->len = ftell(payload_file);
    res->data = malloc(res->len);
    res->install_base = -1;
    res->next = NULL;
    res->prev = NULL;

    rewind(payload_file);
    fread(res->data, 1, res->len, payload_file);
    fclose(payload_file);

    return res;
}

void free_payload(struct payload *p)
{
    free(p->data);
    free(p);
}

long get_address(struct pwned_device *dev, LOCATION_T l)
{
    return 0x180151000;
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
        dev->installed = NULL;
        return CHECKM8_SUCCESS;
    }
    else
    {
        pl->prev->next = pl->next;
        pl->next->prev = pl->prev;
        return CHECKM8_SUCCESS;
    }
}


int install_payload(struct pwned_device *dev, PAYLOAD_T p, LOCATION_T loc)
{
    int i, ret;
    struct payload *pl = get_payload(p);
    long addr = get_address(dev, loc);

    if(pl == NULL || addr == -1) return CHECKM8_FAIL_INVARGS;

    ret = get_device_bundle(dev);
    if(IS_CHECKM8_FAIL(ret)) return ret;

    for(i = 0; i < pl->len; i++)
    {
        ret = dev_memset(dev, addr + i, pl->data[i], 1);
        if(IS_CHECKM8_FAIL(ret))
        {
            release_device_bundle(dev);
            return CHECKM8_FAIL_XFER;
        }
    }

    pl->install_base = addr;
    dev_link_payload(dev, pl);
    release_device_bundle(dev);
    return ret;
}

int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p)
{

}

int execute_payload(struct pwned_device *dev, PAYLOAD_T p, int nargs, ...)
{
    int ret, i;
    struct payload *pl;
    if((pl = dev_retrieve_payload(dev, p)) == NULL) return CHECKM8_FAIL_NOINST;

    ret = get_device_bundle(dev);
    if(IS_CHECKM8_FAIL(ret)) return ret;

    unsigned long long args[nargs + 1];
    args[0] = pl->install_base;

    va_list arg_list;
    va_start(arg_list, nargs);
    for(i = 0; i < nargs; i++)
    {
        args[i + 1] = va_arg(arg_list, unsigned long long);
    }
    va_end(arg_list);

    ret = dev_exec(dev, 16, nargs, args);
    release_device_bundle(dev);
    return ret;
}