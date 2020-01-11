project(checkm8_payloads ASM)
include_directories(include)

set(CMAKE_SYSTEM_PROCESSOR arm)

if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER /usr/bin/aarch64-linux-gnu-as)
    set(CMAKE_OBJCOPY /usr/bin/aarch64-linux-gnu-objcopy)
endif()

set(CMAKE_C_FLAGS "-nostdlib -O")

set(PAYLOADS
        aes
        aes_busy
        aes_sw
        sync
        sysreg
        task_sleep_test)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set_directory_properties(PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/bin/")

foreach(BINARY ${PAYLOADS})
    add_executable(payload_${BINARY} src/${BINARY}.c)
    add_custom_command(TARGET payload_${BINARY} POST_BUILD
                        BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/bin/payload_${BINARY}.bin
                        COMMAND ${CMAKE_OBJCOPY}
                        ARGS -O binary -j .text -j .payload_text -j .payload_data
                            ${CMAKE_CURRENT_BINARY_DIR}/payload_${BINARY}
                            ${CMAKE_CURRENT_SOURCE_DIR}/bin/payload_${BINARY}.bin)
endforeach(BINARY)