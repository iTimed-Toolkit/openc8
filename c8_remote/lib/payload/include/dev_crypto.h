#ifndef CHECKM8_TOOL_CRYPTO_H
#define CHECKM8_TOOL_CRYPTO_H

#include "dev/types.h"

void expand_key(unsigned char key[16], unsigned char key_sched[176], int n);
void aes128_ttable_encrypt_ecb(unsigned char *msg, unsigned char key_sched[176]);

#endif //CHECKM8_TOOL_CRYPTO_H
