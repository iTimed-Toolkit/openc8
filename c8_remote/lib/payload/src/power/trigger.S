.global busy_loop

busy_loop:
    mov     x1,     x0
    mov     x0,     0

_loop:
    add     x0,     x0,     0x1
    cmp     x0,     x1
    b.ne    _loop

    ret