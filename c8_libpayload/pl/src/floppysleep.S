.global fs_routine
.global fs_load
# .global check_subnormal

.section .payload_text, "ax"

fs_load:
    # load from memory
    ldr     s0,         [x0]
    mov     v0.s[1],    v0.s[0]
    mov     v0.s[2],    v0.s[0]
    mov     v0.s[3],    v0.s[0]
    fmov    s31,        1.0
    ucvtf   s30,         w1
    
    mov     v1.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v1.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v1.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v1.s[0],    v30.s[0]
        
    fadd    s30,         s30,     s31
    mov     v2.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v2.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v2.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v2.s[0],    v30.s[0]
        
    fadd    s30,         s30,     s31
    mov     v3.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v3.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v3.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v3.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v4.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v4.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v4.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v4.s[0],    v30.s[0]
        
    fadd    s30,         s30,     s31
    mov     v5.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v5.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v5.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v5.s[0],    v30.s[0]

        
    fadd    s30,         s30,     s31
    mov     v6.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v6.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v6.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v6.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v7.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v7.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v7.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v7.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v8.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v8.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v8.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v8.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v9.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v9.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v9.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v9.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v10.s[3],    v10.s[0]
    fadd    s30,         s30,     s31
    mov     v10.s[2],    v10.s[0]
    fadd    s30,         s30,     s31
    mov     v10.s[1],    v10.s[0]
    fadd    s30,         s30,     s31
    mov     v10.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v11.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v11.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v11.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v11.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v12.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v12.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v12.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v12.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v13.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v13.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v13.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v13.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v14.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v14.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v14.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v14.s[0],    v30.s[0]

    fadd    s30,         s30,     s31
    mov     v15.s[3],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v15.s[2],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v15.s[1],    v30.s[0]
    fadd    s30,         s30,     s31
    mov     v15.s[0],    v30.s[0]

    #mov    s30,        wzr
    #mov    s31,        wzr
    ret

fs_routine:
    fdiv    v16.4s,     v0.4s,      v1.4s
    fdiv    v17.4s,     v0.4s,      v2.4s
    fdiv    v18.4s,     v0.4s,      v3.4s
    fdiv    v19.4s,     v0.4s,      v4.4s
    fdiv    v20.4s,     v0.4s,      v5.4s
    fdiv    v21.4s,     v0.4s,      v6.4s
    fdiv    v22.4s,     v0.4s,      v7.4s
    fdiv    v23.4s,     v0.4s,      v8.4s
    fdiv    v24.4s,     v0.4s,      v9.4s
    fdiv    v25.4s,     v0.4s,      v10.4s
    fdiv    v26.4s,     v0.4s,      v11.4s
    fdiv    v27.4s,     v0.4s,      v12.4s
    fdiv    v28.4s,     v0.4s,      v13.4s
    fdiv    v29.4s,     v0.4s,      v14.4s
    fdiv    v30.4s,     v0.4s,      v15.4s

    fdiv    v16.4s,     v16.4s,     v15.4s
    fdiv    v17.4s,     v17.4s,     v14.4s
    fdiv    v18.4s,     v18.4s,     v13.4s
    fdiv    v19.4s,     v19.4s,     v12.4s
    fdiv    v20.4s,     v20.4s,     v11.4s
    fdiv    v21.4s,     v21.4s,     v10.4s
    fdiv    v22.4s,     v22.4s,     v9.4s
    fdiv    v23.4s,     v23.4s,     v8.4s
    fdiv    v24.4s,     v24.4s,     v7.4s
    fdiv    v25.4s,     v25.4s,     v6.4s
    fdiv    v26.4s,     v26.4s,     v5.4s
    fdiv    v27.4s,     v27.4s,     v4.4s
    fdiv    v28.4s,     v28.4s,     v3.4s
    fdiv    v29.4s,     v29.4s,     v2.4s
    fdiv    v30.4s,     v30.4s,     v1.4s

    mov     w0,         v16.s[3]
    ret
