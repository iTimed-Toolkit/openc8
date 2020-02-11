#ifndef CHECKM8_TOOL_BOOTROM_FUNC_H
#define CHECKM8_TOOL_BOOTROM_FUNC_H

#include "dev_util.h"

/* Crypto */
int hardware_aes(unsigned long long cmd,
                 unsigned char *src, unsigned char *dst,
                 int len, unsigned long long opts,
                 unsigned char *key, unsigned char *iv);

/* Timing */
int clock_gate(int device, int power);
unsigned long long get_time();
void timer_register_int(unsigned long long dl);
void wfi();

/* Tasking */
void *task_new(char *name, BOOTROM_FUNC_I func, void *args, int ssize);
void task_run(void *task);
void task_pause(int usec);
void task_resched();
void task_free(void *task);

void event_new(void *dst, int flags, int state);
void event_notify(void *ev);
void event_wait(void *ev);

/* Heap */
void calc_chksum(unsigned long long *dst, unsigned long long *src, int len, unsigned long long *cookie);
void check_block_chksum(void *ptr);
void check_all_chksums();

void *dev_malloc(int size);
void *dev_memalign(int size, int constr);
void dev_free(void *ptr);

#endif //CHECKM8_TOOL_BOOTROM_FUNC_H
