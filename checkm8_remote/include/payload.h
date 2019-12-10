#ifndef IPWNDFU_REWRITE_C_PAYLOAD_H
#define IPWNDFU_REWRITE_C_PAYLOAD_H

typedef enum
{
    PAYLOAD_AES
} PAYLOAD_T;

struct payload *get_payload(PAYLOAD_T p);

#endif //IPWNDFU_REWRITE_C_PAYLOAD_H
