#include "payload.h"

#include <stdio.h>
#include <stdlib.h>

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
            path = "blehblehbleh";
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
        return PAYLOAD_SUCCESS;
    }
    else if(dev_contains_payload(dev, pl->type) == PAYLOAD_FOUND)
    {
        return PAYLOAD_FAIL_DUP;
    }
    else
    {
        for(curr = dev->installed; curr->next != NULL; curr = curr->next);

        curr->next = pl;
        pl->prev = curr;
        return PAYLOAD_SUCCESS;
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
    struct payload *payload = get_payload(p);
    long addr = get_address(dev, loc);
}

int uninstall_payload(struct pwned_device *dev, PAYLOAD_T p)
{

}

int execute_payload(struct pwned_device *dev, PAYLOAD_T p, ...)
{

}