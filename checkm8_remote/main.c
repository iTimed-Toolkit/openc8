#include <stdio.h>
#include "checkm8.h"

int main()
{
    struct pwned_device *dev = exploit_device();
    if(dev == NULL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

}