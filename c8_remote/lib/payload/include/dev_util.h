#ifndef CHECKM8_TOOL_DEV_UTIL_H
#define CHECKM8_TOOL_DEV_UTIL_H

typedef void (*BOOTROM_FUNC_V)();
typedef int (*BOOTROM_FUNC_I)();
typedef unsigned long long (*BOOTROM_FUNC_ULL)();
typedef void (*(*BOOTROM_FUNC_PTR)());

#define PAYLOAD_SECTION __attribute__ ((section (".payload_text")))
#define TEXT_SECTION    __attribute__ ((section (".text")))
#define BRLIB_SECTION(s)   __attribute__ ((section (".bootrom_lib."s)))

#endif //CHECKM8_TOOL_DEV_UTIL_H
