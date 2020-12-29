#ifndef CHECKM8_TOOL_BOOTROM_FUNC_H
#define CHECKM8_TOOL_BOOTROM_FUNC_H

#include "dev/addr.h"
#include "dev_util.h"
#include "dev_globals.h"
#include "bootrom_types.h"

#define __BOOTROM_CALL__(ret_type, addr, name, args, params) \
    GLOBAL_PTR(__ ## addr, addr) \
    __attribute__ ((section (".payload_data___" #name))) \
    __attribute__ ((always_inline)) \
    static inline ret_type name args { \
    ret_type (*name ## _ptr)() = GET_GLOBAL(__ ## addr); \
    return name ## _ptr params; }

/* Crypto */

/*
 * Interface to the BootROM's hardware crypto accelerator
 *  cmd: encryption direction and mode
 *  src: pointer to the data which to encrypt
 *  dst: pointer at which to store the ciphertext
 *  len: size of data to encrypt
 *  opts: key type and key size
 *  iv: pointer to initialization vector, if CBC mode
 */
__BOOTROM_CALL__(int, ADDR_HARDWARE_AES, hardware_aes,
                 (uint64_t cmd, uint8_t *src, uint8_t *dst,
                         int len, uint64_t opts, uint8_t *key, uint8_t *iv),
                 (cmd, src, dst, len, opts, key, iv))

/*
 * Get random bytes from the BootROM
 *  buf: pointer to fill with random bytes
 *  len: number of random bytes to get
 */
__BOOTROM_CALL__(int, ADDR_GET_RANDOM, get_random,
                 (void *buf, int len), (buf, len))

/*
 * Return entropy from the chip's entropy source
 */
__BOOTROM_CALL__(int, ADDR_GET_ENTROPY, get_entropy,
                 (), ())

/*
 * Calculate the SHA1 hash of a buffer
 *  src: pointer to the data to hash
 *  len: the length of the data
 *  dst: where to store the resulting hash
 */
__BOOTROM_CALL__(void, ADDR_SHA1, sha1,
                 (void *src, int len, void *dst),
                 (src, len, dst))

/* Timing */

/*
 * Turn on/off one of the SoC's peripheral devices
 *  device: the boot_index of the device to clock
 *  power: whether to turn the device on or off
 */
__BOOTROM_CALL__(int, ADDR_CLOCK_GATE, clock_gate,
                 (int device, int power),
                 (device, power))

/*
 * Get a timestamp in microseconds since boot
 */
__BOOTROM_CALL__(uint64_t, ADDR_GET_TIME, get_time,
                 (), ())

/*
 * Get a timestamp in clock ticks since boot
 */
__BOOTROM_CALL__(uint64_t, ADDR_GET_TICKS, get_ticks,
                 (), ())

/*
 * Register an interrupt in the future
 *  dl: number of cycles in which to interrupt
 */
__BOOTROM_CALL__(void, ADDR_TIMER_REGISTER_INT, timer_register_int,
                 (uint64_t dl), (dl, ADDR_RANDOM_RET))

/*
 * Wait for a previously registered interrupt
 */
__BOOTROM_CALL__(void, ADDR_WFI, wfi,
                 (), ())

/* Tasking */

/*
 * Create a new task object (pthread-ish)
 *  name: the name of the task
 *  func: the entry point of the task
 *  args: pointer which will be passed to func
 *  ssize: stack size to allocate
 */
__BOOTROM_CALL__(void *, ADDR_TASK_NEW, task_new,
                 (char *name, void *func, void *args, int ssize),
                 (name, func, args, ssize))

/*
 * Run a previously created task
 *  task: the task to run
 */
__BOOTROM_CALL__(void, ADDR_TASK_RUN, task_run,
                 (void *task), (task))

/*
 * Sleep within the current task
 *  usec: microseconds for which to sleep
 */
__BOOTROM_CALL__(void, ADDR_TASK_PAUSE, task_pause,
                 (int usec), (usec))

/*
 * Schedule another task, if possible
 */
__BOOTROM_CALL__(void, ADDR_TASK_RESCHED, task_resched,
                 (), ())

/*
 * Free a previously allocated task structure
 *  task: the task which to free
 */
__BOOTROM_CALL__(void, ADDR_TASK_FREE, task_free,
                 (void *task), (task))

/*
 * Cleanly exit from the current task
 *  ret: the task's return code
 */
__BOOTROM_CALL__(void, ADDR_TASK_EXIT, task_exit,
                 (int ret), (ret))

/* Events */

/*
 * Create a new (semaphore-like) event
 *  dst: the address at which to make the event
 *  flags: the flags for the event
 *  state: the initial count of the event
 */
__BOOTROM_CALL__(void, ADDR_EVENT_NEW, event_new,
                 (struct event *dst, int flags, int state),
                 (dst, flags, state))

/*
 * Increment the event, signalling any waiting tasks
 *  ev: the event which to signal
 */
__BOOTROM_CALL__(void, ADDR_EVENT_NOTIFY, event_notify,
                 (struct event *ev), (ev))

/*
 * Decrement the event, waiting for a signal
 *  ev: the event on which to wait
 */
__BOOTROM_CALL__(void, ADDR_EVENT_WAIT, event_wait,
                 (struct event *ev), (ev))

/*
 * Try to wait on an event, with a timeout
 *  ev: the event on which to wait
 *  timeout: amount of microseconds to wait before returning
 */
__BOOTROM_CALL__(int, ADDR_EVENT_TRY, event_try,
                 (struct event *ev, int timeout),
                 (ev, timeout))

/*
 * Enter a critical section (disable interrupts)
 */
__BOOTROM_CALL__(void, ADDR_ENTER_CRITCAL, enter_critical,
                 (), ())

/*
 * Exit a critical section (enable interrupts)
 */
__BOOTROM_CALL__(void, ADDR_EXIT_CRITICAL, exit_critical,
                 (), ())

/* Heap */

/*
 * Calculate the metadata checksum for a heap block
 *  dst: where to place the checksum
 *  src: the data which to checksum
 *  len: the length of the data
 *  cookie: pointer to a random global heap constant
 */
__BOOTROM_CALL__(void, ADDR_CALC_CHKSUM, calc_chksum,
                 (void *dst, void *src, int len, void *cookie),
                 (dst, src, len, cookie))

/*
 * Verify the metadata checksum for a heap block
 *  block: the block which to verify
 */
__BOOTROM_CALL__(void, ADDR_CHECK_BLOCK_CHKSUM, check_block_chksum,
                 (void *block), (block))

/*
 * Verify the checksums for all heap blocks
 */
__BOOTROM_CALL__(void, ADDR_CHECK_ALL_CHKSUMS, check_all_chksums,
                 (), ())

/*
 * Allocate some memory from the heap
 *  size: number of bytes to allocate
 */
__BOOTROM_CALL__(void *, ADDR_DEV_MALLOC, dev_malloc,
                 (int size), (size))

/*
 * Allocate some aligned memory from the heap
 *  size: number of bytes to allocate
 *  constr: alignment of the bytes
 */
__BOOTROM_CALL__(void *, ADDR_DEV_MEMALIGN, dev_memalign,
                 (int size, int constr), (size, constr))

/*
 * Free previously allocated heap memory
 *  ptr: memory which to free
 */
__BOOTROM_CALL__(void, ADDR_DEV_FREE, dev_free,
                 (void *ptr), (ptr))

/* Memory */

/*
 * Set the page table base
 *  val: the new page table base
 */
__BOOTROM_CALL__(void, ADDR_WRITE_TTBR0, write_ttbr0,
                 (unsigned long long val), (val))

/*
 * Clean and invalidate a cache line
 *  addr: virtual address to clean and invalidate
 */
__BOOTROM_CALL__(void, ADDR_DC_CIVAC, dc_civac,
                 (void *addr), (addr));

/*
 * Invalidate all TLB entries
 */
__BOOTROM_CALL__(void, ADDR_TLBI, tlbi, (), ())

/*
 * Wait for memory accesses to complete
 */
__BOOTROM_CALL__(void, ADDR_DMB, dmb, (), ())

/*
 * Add a new page mapping to the page table
 *  vaddr: virtual address for the mapping
 *  paddr: physical address for the mappping
 *  size: size of the mapping
 *  perm: permissions for the mapping (rw, rx, etc)
 */
__BOOTROM_CALL__(int, ADDR_MAP_RANGE, map_range,
                 (long long vaddr, long long paddr,
                         int size, page_permission perm),
                 (vaddr, paddr, size, perm))

/* IO */

/*
 * Create a new descriptor for USB
 */
__BOOTROM_CALL__(int, ADDR_CREATE_DESC, usb_create_descriptor,
                 (char *str), (str))

/*
 * Callback to the USB core to complete an IO request
 */
__BOOTROM_CALL__(void, ADDR_USB_CORE_DO_IO, usb_core_do_io,
                 (int ep, void *buf, int len, void *cb),
                 (ep, buf, len, cb))

/*
 * DFU mode data handler
 */
__BOOTROM_CALL__(int, ADDR_HANDLE_INTF_REQ, handle_intf_req,
                 (struct usb_request *req, void **out),
                 (req, out))

__BOOTROM_CALL__(int, ADDR_GPIO_READ, gpio_read,
                 (int num), (num))

__BOOTROM_CALL__(void, ADDR_GPIO_WRITE, gpio_write,
                 (int num, int val), (num, val))

/* Standard */

/*
 * Copy memory
 *  dst: where to copy to
 *  src: where to copy from
 *  len: number of bytes to copy
 */
__BOOTROM_CALL__(void, ADDR_DEV_MEMCPY, dev_memcpy,
                 (void *dst, void *src, int len),
                 (dst, src, len))

/*
 * Set memory to a constant
 *  dst: memory to set
 *  c: value for the memory
 *  len: number of bytes to set
 */
__BOOTROM_CALL__(void, ADDR_DEV_MEMSET, dev_memset,
                 (void *dst, char c, int len),
                 (dst, c, len))

/*
 * Calculate the length of a string
 *  str: the string to return the length of
 */
__BOOTROM_CALL__(int, ADDR_DEV_STRLEN, dev_strlen,
                 (char *str), (str))

#endif //CHECKM8_TOOL_BOOTROM_FUNC_H
