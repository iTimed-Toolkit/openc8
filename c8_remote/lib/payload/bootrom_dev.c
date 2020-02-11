#include "bootrom_addr.h"
#include "dev_util.h"

/* Crypto */
BRLIB_SECTION("crypto")
int hardware_aes(unsigned long long cmd,
                 unsigned char *src, unsigned char *dst,
                 int len, unsigned long long opts,
                 unsigned char *key, unsigned char *iv)
{
    return ((BOOTROM_FUNC_I) ADDR_HARDWARE_AES)(cmd, src, dst, len, opts, key, iv);
}

/* Timing */
BRLIB_SECTION("timing.power")
int clock_gate(int device, int power)
{
    return ((BOOTROM_FUNC_I) ADDR_CLOCK_GATE)(device, power);
}

BRLIB_SECTION("timing.time")
unsigned long long get_time()
{
    return ((BOOTROM_FUNC_ULL) ADDR_GET_TIME)();
}

BRLIB_SECTION("timing.sleep")
void timer_register_int(unsigned long long dl)
{
    ((BOOTROM_FUNC_V) ADDR_TIMER_REGISTER_INT)(dl, ADDR_RANDOM_RET);
}

BRLIB_SECTION("timing.sleep")
void wfi()
{
    ((BOOTROM_FUNC_V) ADDR_WFI)();
}

/* Tasking */
BRLIB_SECTION("tasking.task")
void *task_new(char *name, BOOTROM_FUNC_I func, void *args, int ssize)
{
    return ((BOOTROM_FUNC_PTR) ADDR_TASK_NEW)(name, func, args, ssize);
}

void task_run(void *task)
{
    ((BOOTROM_FUNC_V) ADDR_TASK_RUN)(task);
}

BRLIB_SECTION("tasking.task")
void task_pause(int usec)
{
    ((BOOTROM_FUNC_V) ADDR_TASK_PAUSE)(usec);
}

BRLIB_SECTION("tasking.task")
void task_resched()
{
    ((BOOTROM_FUNC_V) ADDR_TASK_RESCHED)();
}

BRLIB_SECTION("tasking.task")
void task_free(void *task)
{
    ((BOOTROM_FUNC_V) ADDR_TASK_FREE)(task);
}

BRLIB_SECTION("tasking.event")
void event_new(void *dst, int flags, int state)
{
    ((BOOTROM_FUNC_V) ADDR_EVENT_NEW)(dst, flags, state);
}

BRLIB_SECTION("tasking.event")
void event_notify(void *ev)
{
    ((BOOTROM_FUNC_V) ADDR_EVENT_NOTIFY)(ev);
}

BRLIB_SECTION("tasking.event")
void event_wait(void *ev)
{
    ((BOOTROM_FUNC_V) ADDR_EVENT_WAIT)(ev);
}

/* Heap */
BRLIB_SECTION("heap.mgmt")
void calc_chksum(unsigned long long *dst, unsigned long long *src,
                 int len, unsigned long long *cookie)
{
    ((BOOTROM_FUNC_V) ADDR_CALC_CHKSUM)(dst, src, len, cookie);
}

BRLIB_SECTION("heap.mgmt")
void check_block_chksum(void *ptr)
{
    ((BOOTROM_FUNC_V) ADDR_CHECK_BLOCK_CKSUM)(ptr);
}

BRLIB_SECTION("heap.mgmt")
void check_all_chksums()
{
    ((BOOTROM_FUNC_V) ADDR_CHECK_ALL_CHKSUMS)();
}

BRLIB_SECTION("heap.alloc")
void *dev_malloc(int size)
{
    return ((BOOTROM_FUNC_PTR) ADDR_DEV_MALLOC)(size);
}

BRLIB_SECTION("heap.alloc")
void *dev_memalign(int size, int constr)
{
    return ((BOOTROM_FUNC_PTR) ADDR_DEV_MEMALIGN)(size, constr);
}

BRLIB_SECTION("heap.alloc")
void dev_free(void *ptr)
{
    ((BOOTROM_FUNC_PTR) ADDR_DEV_FREE)(ptr);
}