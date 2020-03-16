.global get_hook
.global get_

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
    // proceed to boot
    bl      get_en_boot_intf
    mov     x4,     x0
    bl      get_boot_entry
    mov     x5,     x0

    // disable the DFU boot interface
    mov     x0,     0
    mov     x1,     x22
    mov     x2,     x23
    blr     x4

    mov     x19,    0
    blr     x5
