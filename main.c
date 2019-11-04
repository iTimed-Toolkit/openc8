#include <stdio.h>
#include "exploit/checkm8.h"

int main()
{
    int status = exploit_device();
    if(status == 0)
    {
        printf("Successfully exploited device!\n");
    }
    else
    {
        printf("Failed to exploit device\n");
    }
}