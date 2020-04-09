#ifndef CHECKM8_TOOL_CRYPTO_H
#define CHECKM8_TOOL_CRYPTO_H

#include "dev/types.h"

void expand_key_sbox(unsigned char key[16], unsigned char key_sched[176],
                     int n, struct aes_sbox_constants *c);

void expand_key_ttable(unsigned char key[16], unsigned char key_sched[176],
                       int n, struct aes_ttable_constants *c);

void aes128_sbox_encrypt_ecb(unsigned char *msg,
                        unsigned char key_sched[176], struct aes_sbox_constants *c);

void aes128_ttable_encrypt_ecb(unsigned char *msg,
                               unsigned char key_sched[176], struct aes_ttable_constants *c);

#endif //CHECKM8_TOOL_CRYPTO_H
