.global run

.section .payload_text, "ax"
run:
    mov     x0,     0x0
    mov     x1,     0x0
    mov     x2,     0x0
    mov     x3,     0x0
    mov     x4,     0x0
    mov     x5,     0x0
    mov     x6,     0x0
    mov     x7,     0x0
    mov     x8,     0x0

_loop:
    cmp     x0,     0xFFF000
    b.eq    _done
    and     x9,     x0,     0x7

    cmp     x9,     0x0
    b.eq    _inc_0
    cmp     x9,     0x1
    b.eq    _inc_1
    cmp     x9,     0x2
    b.eq    _inc_2
    cmp     x9,     0x3
    b.eq    _inc_3
    cmp     x9,     0x4
    b.eq    _inc_4
    cmp     x9,     0x5
    b.eq    _inc_5
    cmp     x9,     0x6
    b.eq    _inc_6
    b       _inc_7

_inc_0:
    add     x1,     x1,     x0
    add     x0,     x0,     0x1
    b       _loop

_inc_1:
    add     x2,     x2,     x0
    add     x0,     x0,     0x1
    b       _loop

_inc_2:
    add     x3,     x3,     x0
    add     x0,     x0,     0x1
    b       _loop

_inc_3:
    add     x4,     x4,     x0
    add     x0,     x0,     0x1
    b       _loop

_inc_4:
    add     x5,     x5,     x0
    add     x0,     x0,     0x1
    b       _loop

_inc_5:
    add     x6,     x6,     x0
    add     x0,     x0,     0x1
    b       _loop

_inc_6:
    add     x7,     x7,     x0
    add     x0,     x0,     0x1
    b       _loop

_inc_7:
    add     x8,     x8,     x0
    add     x0,     x0,     0x1
    b       _loop

_done:
    mov     x0,     0x0
    ret
