#ifndef CHECKM8_TOOL_CRYPTO_H
#define CHECKM8_TOOL_CRYPTO_H

#include "dev/types.h"

void expand_key(unsigned char key[16], unsigned char key_sched[176],
                int n, struct aes_constants *c);

void aes128_encrypt_ecb(unsigned char *msg, unsigned int msg_len,
                        unsigned char key_sched[176], struct aes_constants *c);


#endif //CHECKM8_TOOL_CRYPTO_H
