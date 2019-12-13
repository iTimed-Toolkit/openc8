struct sysregs
{
    long pt_base;
    long evt_base;
};

struct sysregs _start()
{
    struct sysregs res;
    __asm__("mrs %0, ttbr1_el1" : "=r" (res.pt_base));
    __asm__("mrs %0, vbar_el1"  : "=r" (res.evt_base));

    return res;
}