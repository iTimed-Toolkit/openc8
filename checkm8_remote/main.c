#include "checkm8.h"
#include "payload.h"

#include <stdio.h>
#include <stdarg.h>
#include <execinfo.h>

void checkm8_debug(const char *format, ...)
{
#ifdef CHECKM8_LOGGING
    void *traces[100];
    int depth = backtrace(traces, 100) - 5;
    for(int i = 0; i < depth; i++)
    {
        printf("\t");
    }

    va_list args;

    va_start (args, format);
    vprintf(format, args);
    va_end(args);
#endif
}

int main()
{
    struct pwned_device *dev = exploit_device();
    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    install_payload(dev, PAYLOAD_SYSREG, DRAM);
    execute_payload(dev, PAYLOAD_SYSREG, 0);
}