//
// Created by grg on 11/13/20.
//

#ifndef CHECKM8_TOOL_ARMV8_H
#define CHECKM8_TOOL_ARMV8_H

typedef enum
{
    X0 = 0, X1, X2, X3, X4, X5, X6, X7,
    X8, X9, X10, X11, X12, X13, X14, X15,
    X16, X17, X18, X19, X20, X21, X22, X23,
    X24, X25, X26, X27, X28, X29, X30, X31
} armv8_reg_t;

#define __SHIFT_0       0u
#define __SHIFT_16      1u
#define __SHIFT_32      2u
#define __SHIFT_48      3u

/* Data */

#define __OP_MOVK           (uint32_t)  (0b111100101u << 23u)
#define __MOVK_HW(shift)    (uint8_t)   ( __SHIFT_ ## shift << 21u)
#define __MOVK_IMM(imm)     (uint16_t)  (imm << 5u)
#define __MOVK_DST(dst)     (uint8_t)   (dst)

#define MOVK(dst, imm, shift) \
        __OP_MOVK | __MOVK_HW(shift) | __MOVK_IMM(imm) | __MOVK_DST(dst)

__attribute__ ((always_inline))
inline uint32_t movk(armv8_reg_t reg, uint16_t imm, uint8_t shift)
{
    if(reg >= X0 && reg <= X31 &&
       shift % 16 == 0 && shift <= 48)
        return __OP_MOVK | ((shift / 4u) & 0b11u) << 21u | __MOVK_IMM(imm) | (reg & 0b11111u);
    else
        return -1;
}

#define __OP_MOVZ           (uint32_t)  (0b110100101u << 23u)
#define __MOVZ_HW(shift)    __MOVK_HW(shift)
#define __MOVZ_IMM(imm)     __MOVK_IMM(imm)
#define __MOVZ_DST(dst)     __MOVK_DST(dst)

#define MOVZ(dst, imm, shift) \
        __OP_MOVZ | __MOVZ_HW(shift) | __MOVZ_IMM(imm) | __MOVZ_DST(dst)

__attribute__ ((always_inline))
inline uint32_t movz(armv8_reg_t reg, uint16_t imm, uint8_t shift)
{
    if(reg >= X0 && reg <= X31 &&
       shift % 16 == 0 && shift <= 48)
        return __OP_MOVZ | ((shift / 4u) & 0b11u) << 21u | __MOVZ_IMM(imm) | (reg & 0b11111u);
    else
        return -1;
}

/* Control Flow */

#define __OP_BR             (uint32_t)  (0b1101011000011111000000u << 10u)
#define __BR_DST(dst)       (uint8_t)   (dst << 5u)

#define BR(dst) \
        __OP_BR | __BR_DST(dst)

__attribute ((always_inline))
inline uint32_t br(armv8_reg_t dst)
{
    return __OP_BR | __BR_DST(dst);
}

#endif //CHECKM8_TOOL_ARMV8_H
