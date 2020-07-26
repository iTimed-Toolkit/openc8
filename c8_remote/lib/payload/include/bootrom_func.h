#ifndef CHECKM8_TOOL_BOOTROM_FUNC_H
#define CHECKM8_TOOL_BOOTROM_FUNC_H

#include "dev/addr.h"
#include "dev/types.h"
#include "dev_util.h"
#include "dev_globals.h"

#define __BOOTROM_CALL__(ret_type, addr, name, args, params) \
    GLOBAL_PTR(__ ## addr, addr) \
    __attribute__ ((section (".payload_data___" #name))) \
    __attribute__ ((always_inline)) \
    static inline ret_type name args { \
    ret_type (*name ## _ptr)() = GET_GLOBAL(__ ## addr); \
    return name ## _ptr params; }

/* Crypto */

__BOOTROM_CALL__(int, ADDR_HARDWARE_AES, hardware_aes,
                 (uint64_t cmd, uint8_t * src, uint8_t * dst,
                         int len, uint64_t opts, uint8_t * key, uint8_t * iv),
                 (cmd, src, dst, len, opts, key, iv))

__BOOTROM_CALL__(int, ADDR_GET_RANDOM, get_random,
                 (void *buf, int len), (buf, len))


__BOOTROM_CALL__(int, ADDR_GET_ENTROPY, get_entropy,
                 (), ())

__BOOTROM_CALL__(void, ADDR_SHA1, sha1,
                 (void *src, int len, void *dst),
                 (src, len, dst))

/* Timing */

__BOOTROM_CALL__(int, ADDR_CLOCK_GATE, clock_gate,
                 (int device, int power),
                 (device, power))

__BOOTROM_CALL__(uint64_t, ADDR_GET_TIME, get_time,
                 (), ())

__BOOTROM_CALL__(uint64_t, ADDR_GET_TICKS, get_ticks,
                 (), ())

__BOOTROM_CALL__(void, ADDR_TIMER_REGISTER_INT, timer_register_int,
                 (uint64_t dl), (dl))

__BOOTROM_CALL__(void, ADDR_WFI, wfi,
                 (), ())

/* Tasking */

__BOOTROM_CALL__(void *, ADDR_TASK_NEW, task_new,
                 (char *name, void * func, void *args, int ssize),
                 (name, func, args, ssize))

__BOOTROM_CALL__(void, ADDR_TASK_RUN, task_run,
                 (void *task), (task))

__BOOTROM_CALL__(void, ADDR_TASK_PAUSE, task_pause,
                 (int usec), (usec))

__BOOTROM_CALL__(void, ADDR_TASK_RESCHED, task_resched,
                 (), ())

__BOOTROM_CALL__(void, ADDR_TASK_FREE, task_free,
                 (void *task), (task))

__BOOTROM_CALL__(void, ADDR_TASK_EXIT, task_exit,
                 (int ret), (ret))

/* Events */

__BOOTROM_CALL__(void, ADDR_EVENT_NEW, event_new,
                 (struct event *dst, int flags, int state),
                 (dst, flags, state))

__BOOTROM_CALL__(void, ADDR_EVENT_NOTIFY, event_notify,
                 (struct event *ev), (ev))

__BOOTROM_CALL__(void, ADDR_EVENT_WAIT, event_wait,
                 (struct event *ev), (ev))

__BOOTROM_CALL__(int, ADDR_EVENT_TRY, event_try,
                 (struct event *ev, int timeout),
                 (ev, timeout))

__BOOTROM_CALL__(void, ADDR_ENTER_CRITCAL, enter_critical,
                 (), ())

__BOOTROM_CALL__(void, ADDR_EXIT_CRITICAL, exit_critical,
                 (), ())

/* Heap */

__BOOTROM_CALL__(void, ADDR_CALC_CHKSUM, calc_chksum,
                 (void *dst, void *src, int len, void *cookie),
                 (dst, src, len, cookie))

__BOOTROM_CALL__(void, ADDR_CHECK_BLOCK_CHKSUM, check_block_chksum,
                 (void *block), (block))

__BOOTROM_CALL__(void, ADDR_CHECK_ALL_CHKSUMS, check_all_chksums,
                 (), ())

/* USB */

__BOOTROM_CALL__(int, ADDR_CREATE_DESC, usb_create_descriptor,
                 (char *str), (str))

__BOOTROM_CALL__(void, ADDR_USB_CORE_DO_IO, usb_core_do_io,
                 (int ep, void *buf, int len, void *cb),
                 (ep, buf, len, cb))

__BOOTROM_CALL__(int, ADDR_HANDLE_INTF_REQ, handle_intf_req,
                 (struct usb_request *req, void **out),
                 (req, out))

/* Standard */

__BOOTROM_CALL__(void *, ADDR_DEV_MALLOC, dev_malloc,
                 (long long size), (size))

__BOOTROM_CALL__(void *, ADDR_DEV_MEMALIGN, dev_memalign,
                 (int size, int constr), (size, constr))

__BOOTROM_CALL__(void, ADDR_DEV_FREE, dev_free,
                 (void *ptr), (ptr))

__BOOTROM_CALL__(void, ADDR_DEV_MEMCPY, dev_memcpy,
                 (void *dst, void *src, long long len),
                 (dst, src, len))

__BOOTROM_CALL__(int, ADDR_DEV_STRLEN, dev_strlen,
                 (char *str), (str))

#endif //CHECKM8_TOOL_BOOTROM_FUNC_H
