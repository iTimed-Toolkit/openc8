enable_language(ASM)
include_directories(include)

set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
set(CMAKE_ASM_COMPILER /usr/bin/aarch64-linux-gnu-as)
set(CMAKE_OBJCOPY /usr/bin/aarch64-linux-gnu-objcopy)
set(CMAKE_C_FLAGS "-nostdlib")

add_executable(payload aes.c)