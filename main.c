#include <stdio.h>
#include "exploit/checkm8.h"

int main()
{
    int status = exploit_device();
    if(status != 0)
    {
        printf("Failed to exploit device\n");
        return status;
    }
    else
    {
        unsigned char aes_in[16] = {0xDE, 0xAD, 0xBE, 0xEF,
                                    0xDE, 0xAD, 0xBE, 0xEF,
                                    0xDE, 0xAD, 0xBE, 0xEF,
                                    0xDE, 0xAD, 0xBE, 0xEF};
        unsigned char aes_out[16];

        aes(aes_in, aes_out, AES_ENCRYPT, AES_UID_KEY);
        printf("%s\n", aes_out);
    }
}