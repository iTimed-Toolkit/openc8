#ifndef CHECKM8_TOOL_BOOTROM_FUNC_H
#define CHECKM8_TOOL_BOOTROM_FUNC_H

#include "dev/addr.h"
#include "dev/types.h"
#include "dev_util.h"

/* Crypto */
static inline int hardware_aes(unsigned long long cmd,
                        unsigned char *src, unsigned char *dst,
                        int len, unsigned long long opts,
                        unsigned char *key, unsigned char *iv)
{
    return ((BOOTROM_FUNC_I) ADDR_HARDWARE_AES)(cmd, src, dst, len, opts, key, iv);
}

static inline int get_random(void *buf, int len)
{
    return ((BOOTROM_FUNC_I) ADDR_GET_RANDOM)(buf, len);
}

static inline unsigned int get_entropy()
{
    return ((BOOTROM_FUNC_I) ADDR_GET_ENTROPY)();
}

static inline void sha1(void *src, int len, void *dst)
{
    return ((BOOTROM_FUNC_V) ADDR_SHA1)(src, len, dst);
}

/* Timing */
static inline int clock_gate(int device, int power)
{
    return ((BOOTROM_FUNC_I) ADDR_CLOCK_GATE)(device, power);
}

static inline unsigned long long get_time()
{
    return ((BOOTROM_FUNC_ULL) ADDR_GET_TIME)();
}

static inline unsigned long long get_ticks()
{
    return ((BOOTROM_FUNC_ULL) ADDR_GET_TICKS)();
}

static inline void timer_register_int(unsigned long long dl)
{
    ((BOOTROM_FUNC_V) ADDR_TIMER_REGISTER_INT)(dl, ADDR_RANDOM_RET);
}

static inline void wfi()
{
    ((BOOTROM_FUNC_V) ADDR_WFI)();
}

/* Tasking */
static inline void *task_new(char *name, BOOTROM_FUNC_I func, void *args, int ssize)
{
    return ((BOOTROM_FUNC_PTR) ADDR_TASK_NEW)(name, func, args, ssize);
}

static inline void task_run(void *task)
{
    ((BOOTROM_FUNC_V) ADDR_TASK_RUN)(task);
}

static inline void task_pause(int usec)
{
    ((BOOTROM_FUNC_V) ADDR_TASK_PAUSE)(usec);
}

static inline void task_resched()
{
    ((BOOTROM_FUNC_V) ADDR_TASK_RESCHED)();
}

static inline void task_free(void *task)
{
    ((BOOTROM_FUNC_V) ADDR_TASK_FREE)(task);
}

static inline void event_new(struct event *dst, int flags, int state)
{
    ((BOOTROM_FUNC_V) ADDR_EVENT_NEW)(dst, flags, state);
}

static inline void event_notify(struct event *ev)
{
    ((BOOTROM_FUNC_V) ADDR_EVENT_NOTIFY)(ev);
}

static inline void event_wait(struct event *ev)
{
    ((BOOTROM_FUNC_V) ADDR_EVENT_WAIT)(ev);
}

static inline int event_try(struct event *ev, int timeout)
{
    return ((BOOTROM_FUNC_I) ADDR_EVENT_TRY)(ev, timeout);
}

/* Heap */
static inline void calc_chksum(unsigned long long *dst, unsigned long long *src,
                        int len, unsigned long long *cookie)
{
    ((BOOTROM_FUNC_V) ADDR_CALC_CHKSUM)(dst, src, len, cookie);
}

static inline void check_block_chksum(void *ptr)
{
    ((BOOTROM_FUNC_V) ADDR_CHECK_BLOCK_CKSUM)(ptr);
}

static inline void check_all_chksums()
{
    ((BOOTROM_FUNC_V) ADDR_CHECK_ALL_CHKSUMS)();
}

static inline void *dev_malloc(int size)
{
    return ((BOOTROM_FUNC_PTR) ADDR_DEV_MALLOC)(size);
}

static inline void *dev_memalign(int size, int constr)
{
    return ((BOOTROM_FUNC_PTR) ADDR_DEV_MEMALIGN)(size, constr);
}

static inline void dev_free(void *ptr)
{
    ((BOOTROM_FUNC_PTR) ADDR_DEV_FREE)(ptr);
}

#endif //CHECKM8_TOOL_BOOTROM_FUNC_H
