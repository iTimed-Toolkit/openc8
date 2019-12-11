#include "payload.h"

#include <stdio.h>
#include <stdlib.h>
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
            path = "/home/grg/Projects/School/NCSU/iphone_aes_sc/ipwndfu_rewrite_c/checkm8_remote/bin/payloads/payload_aes.bin";
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

}

long get_address(struct pwned_device *dev, LOCATION_T l)
{
    return 0x180151000;
}


int dev_contains_payload(struct pwned_device *dev, PAYLOAD_T p)
{
    struct payload *curr;
    for(curr = dev->installed; curr != NULL; curr = curr->next)
    {
        if(curr->type == p) return PAYLOAD_FOUND;
    }

    return PAYLOAD_NOT_FOUND;
}

int dev_insert_payload(struct pwned_device *dev, struct payload *pl)
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

struct payload *dev_remove_payload(struct pwned_device *dev, PAYLOAD_T p)
{
    struct payload *curr;
    if(dev->installed == NULL)
    {
        return NULL;
    }
    else
    {
        for(curr = dev->installed; curr != NULL; curr = curr->next)
        {
            if(curr->type == p)
            {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                return curr;
            }
        }
    }

    return NULL;
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

    dev_insert_payload(dev, pl);
    release_device_bundle(dev);
    return ret;
}

int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p)
{

}

int execute_payload(struct pwned_device *dev, PAYLOAD_T p, ...)
{

}