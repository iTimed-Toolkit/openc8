enable_language(ASM)
include_directories(include)

set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
set(CMAKE_ASM_COMPILER /usr/bin/aarch64-linux-gnu-as)
set(CMAKE_OBJCOPY /usr/bin/aarch64-linux-gnu-objcopy)
set(CMAKE_C_FLAGS "-nostdlib -O")

set(PAYLOADS payload_aes payload_sysreg)
add_executable(payload_aes src/aes.c)
add_executable(payload_sysreg src/sysreg.c)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
foreach(BINARY ${PAYLOADS})
    add_custom_command(TARGET ${BINARY} POST_BUILD
                        BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BINARY}.bin
                        COMMAND ${CMAKE_OBJCOPY}
                        ARGS -O binary -j .text -j .payload_text
                            ${CMAKE_CURRENT_BINARY_DIR}/${BINARY}
                            ${CMAKE_CURRENT_SOURCE_DIR}/bin/${BINARY}.bin)
endforeach(BINARY)