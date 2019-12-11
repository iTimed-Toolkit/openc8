#include <stdio.h>
#include "checkm8.h"
#include "payload.h"

int main()
{
    struct pwned_device *dev = exploit_device();
    if(dev == NULL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    install_payload(dev, PAYLOAD_AES, DRAM);
}