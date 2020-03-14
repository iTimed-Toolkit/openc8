.global get_hook

.section .payload_text, "ax"

// get the address of the boot hook function
get_hook:
    mov     x1,     x30
    b       _boot_thunk

_hook_ret:
    mov     x0,     x30
    mov     x30,    x1
    ret


// actual execution
_boot_thunk:
    bl      _hook_ret

_boot_func:
    stp     x26,    x26,    [sp, -0x50]!
    stp     x24,    x23,    [sp, 0x10]
    stp     x22,    x21,    [sp, 0x20]
    stp     x20,    x19,    [sp, 0x30]
    stp     x29,    x30,    [sp, 0x40]
    add     x29,    sp,     0x40

    // call the patch function
    bl      patch

    // then proceed to boot
    bl      get_en_boot_intf
    mov     x4,     x0
    bl      get_boot_entry
    mov     x5,     x1

    // disable the DFU boot interface
    mov     x0,     0
    mov     x1,     x22
    mov     x2,     x23
    blr     x4

    // start NVME boot
    ldp     x29,    x30,    [sp, 0x40]
    ldp     x20,    x19,    [sp, 0x30]
    ldp     x22,    x21,    [sp, 0x20]
    ldp     x24,    x23,    [sp, 0x10]
    ldp     x26,    x25,    [sp],   0x50

    mov     x19,    0
    blr     x5
