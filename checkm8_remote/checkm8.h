#ifndef IPWNDFU_REWRITE_C_CHECKM8_H
#define IPWNDFU_REWRITE_C_CHECKM8_H

int exploit_device();

#define AES_ENCRYPT 16
#define AES_DECRYPT 17

#define AES_GID_KEY 0x2000200
#define AES_UID_KEY 0x2000201

int aes(unsigned char *source, unsigned char *target, int encrypt, int key);

#endif //IPWNDFU_REWRITE_C_CHECKM8_H
