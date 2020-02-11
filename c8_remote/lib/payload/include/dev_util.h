#ifndef CHECKM8_TOOL_DEV_UTIL_H
#define CHECKM8_TOOL_DEV_UTIL_H

typedef void (*BOOTROM_FUNC_V)();
typedef int (*BOOTROM_FUNC_I)();
typedef unsigned long long (*BOOTROM_FUNC_ULL)();
typedef void (*(*BOOTROM_FUNC_PTR)());

typedef char                int8_t;
typedef short               int16_t;
typedef int                 int32_t;
typedef long long           int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

#define PAYLOAD_SECTION __attribute__ ((section (".payload_text")))
#define TEXT_SECTION    __attribute__ ((section (".text")))

#endif //CHECKM8_TOOL_DEV_UTIL_H
