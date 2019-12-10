#include "payload.h"

#include <stdio.h>
#include <stdlib.h>

struct payload
{
    char *path;
    unsigned char *data;
    long len;
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
    res->path = path;
    res->len = ftell(payload_file);
    res->data = malloc(res->len);

    rewind(payload_file);
    fread(res->data, 1, res->len, payload_file);
    fclose(payload_file);

    return res;
}