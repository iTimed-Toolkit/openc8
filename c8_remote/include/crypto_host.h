#ifndef CHECKM8_TOOL_CRYPTO_HOST_H
#define CHECKM8_TOOL_CRYPTO_HOST_H

#include "bootrom_type.h"

void expand_key(unsigned char key[16], unsigned char key_sched[176],
                int n, struct aes_constants *c);

void aes128_encrypt_ecb(unsigned char *msg, unsigned int msg_len,
                        unsigned char key_sched[176], struct aes_constants *c);

struct aes_constants *get_constants();

#endif //CHECKM8_TOOL_CRYPTO_HOST_H
