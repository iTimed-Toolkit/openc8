#ifndef CHECKM8_TOOL_DEV_GLOBALS_H
#define CHECKM8_TOOL_DEV_GLOBALS_H

#include "dev/shared_types.h"

/* Constructors */

#define __PL_GBL_INIT__(name, asm_type, ...) \
    __asm__  (".global " #name "\n"  \
                ".section .payload_data_" #name ", \"ax\"\n " \
                ".align 2\n" \
                #name ":\n" \
                #asm_type " " #__VA_ARGS__ "\n"); \

#define __PL_GBL_ATTR__(name) \
    __attribute__ ((section (".payload_data_" #name))) \
    __attribute__ ((always_inline))

#define __PL_GBL_PTR_FUNC__(name, type) \
    __PL_GBL_ATTR__(name) \
    static inline type *name ## _ptr() { \
    type *addr; \
    __asm__ ("adr %0, " #name : "=r" (addr)); \
    return addr; }

// todo: could potentially optimize reads to one ldr instruction
#define __PL_GBL_RD_FUNC__(name, type) \
    __PL_GBL_ATTR__(name) \
    static inline type name ## _rd() { \
    return *name ## _ptr(); }

#define __PL_GBL_ARR_RD_FUNC__(name, type) \
    __PL_GBL_ATTR__(name) \
    static inline type name ## _rd(int i) { \
    return name ## _ptr()[i]; }

// todo: these could probably be optimized
#define __PL_GBL_WR_FUNC__(name, type) \
    __PL_GBL_ATTR__(name) \
    static inline void name ## _wr(type v) { \
    *(type *) (WRITEABLE_SRAM((uint64_t) name ## _ptr())) = v; }

#define __PL_GBL_ARR_WR_FUNC__(name, type) \
    __PL_GBL_ATTR__(name) \
    static inline void name ## _wr(int i, type v) { \
    ((type *)(WRITEABLE_SRAM((uint64_t) name ## _ptr())))[i] = v; }

#define __PL_GBL_STD_FUNCS__(name, type) \
    __PL_GBL_PTR_FUNC__(name, type) \
    __PL_GBL_RD_FUNC__(name, type) \
    __PL_GBL_WR_FUNC__(name, type) \

#define __PL_GBL_ARR_FUNCS__(name, type) \
    __PL_GBL_PTR_FUNC__(name, type) \
    __PL_GBL_ARR_RD_FUNC__(name, type) \
    __PL_GBL_ARR_WR_FUNC__(name, type)

#define __CHECK_STRUCT_SIZE__(cond) \
    void checker(){ \
    ((void) sizeof(char[1-2*!!(cond)])); }

/* Declaration */

#define GLOBAL_CHAR(name, value) \
    __PL_GBL_INIT__(name, ".byte", value) \
    __PL_GBL_STD_FUNCS__(name, char)

#define GLOBAL_CHAR_ARR(name, ...) \
    __PL_GBL_INIT__(name, ".byte", __VA_ARGS__) \
    __PL_GBL_ARR_FUNCS__(name, char)

#define GLOBAL_SHORT(name, value) \
    __PL_GBL_INIT__(name, ".hword", value) \
    __PL_GBL_STD_FUNCS__(name, short)

#define GLOBAL_SHORT_ARR(name, ...) \
    __PL_GBL_INIT__(name, ".hword", __VA_ARGS__) \
    __PL_GBL_ARR_FUNCS__(name, short)

#define GLOBAL_INT(name, value) \
    __PL_GBL_INIT__(name, ".word", value) \
    __PL_GBL_STD_FUNCS__(name, int)

#define GLOBAL_INT_ARR(name, ...) \
    __PL_GBL_INIT__(name, ".word", __VA_ARGS__) \
    __PL_GBL_ARR_FUNCS__(name, int)

#define GLOBAL_LONG(name, value) \
    __PL_GBL_INIT__(name, ".quad", value) \
    __PL_GBL_STD_FUNCS__(name, long long)

#define GLOBAL_LONG_ARR(name, ...) \
    __PL_GBL_INIT__(name, ".quad", __VA_ARGS__) \
    __PL_GBL_ARR_FUNCS__(name, long long)

#define GLOBAL_STR(name, value) \
    __PL_GBL_INIT__(name, ".asciz", value) \
    __PL_GBL_ARR_FUNCS__(name, char)

#define GLOBAL_PTR(name, value) \
    __PL_GBL_INIT__(name, ".quad", value) \
    __PL_GBL_STD_FUNCS__(name, void *)

#define GLOBAL_STRUCT(name, type, size) \
    __CHECK_STRUCT_SIZE__(sizeof(type) == size - 1) \
    __PL_GBL_INIT__(name, ".skip", size) \
    __PL_GBL_PTR_FUNC__(name, type)

/* Getters */

#define GET_PTR_TO_GLOBAL(name) \
    name ## _ptr()

#define GET_GLOBAL(name) \
    name ## _rd()

#define GET_GLOBAL_AT(name, index) \
    name ## _rd(index)

/* Setters */

#define SET_GLOBAL(name, value) \
    name ## _wr(value)

#endif //CHECKM8_TOOL_DEV_GLOBALS_H
