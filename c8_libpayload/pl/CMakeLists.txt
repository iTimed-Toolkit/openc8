project(checkm8_libpayload_sources C ASM)
include_directories(include)

set(CMAKE_SYSTEM_PROCESSOR arm)
if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(CMAKE_C_COMPILER   /usr/bin/aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER /usr/bin/aarch64-linux-gnu-as)
    set(CMAKE_OBJCOPY      /usr/bin/aarch64-linux-gnu-objcopy)
endif()

set(CMAKE_C_FLAGS "-nostdlib -O")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
foreach(NAME ${PL_NAMES})
    add_executable(payload_${NAME} src/${NAME}.c)
    add_custom_command(TARGET       payload_${NAME} POST_BUILD
                        BYPRODUCTS  ${CMAKE_CURRENT_BINARY_DIR}/bin/payload_${NAME}.bin
                        COMMAND     ${CMAKE_OBJCOPY}
                        ARGS        -O binary -j .text -j .payload_text -j .payload_data
                                    ${CMAKE_CURRENT_BINARY_DIR}/payload_${NAME}
                                    ${CMAKE_CURRENT_BINARY_DIR}/bin/payload_${NAME}.bin)
endforeach(NAME)