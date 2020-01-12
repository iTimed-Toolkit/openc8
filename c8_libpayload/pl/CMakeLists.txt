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
foreach(PL ${PL_NAMES_SHORT})
    add_executable(payload_${PL} src/${PL}.c)
    add_custom_command(TARGET       payload_${PL} POST_BUILD
                        BYPRODUCTS  ${CMAKE_CURRENT_BINARY_DIR}/bin/payload_${PL}.bin
                        COMMAND     ${CMAKE_OBJCOPY}
                        ARGS        -O binary -j .text -j .payload_text -j .payload_data
                                    ${CMAKE_CURRENT_BINARY_DIR}/payload_${PL}
                                    ${CMAKE_CURRENT_BINARY_DIR}/bin/payload_${PL}.bin)

    add_custom_target(payload_${PL}_bin DEPENDS payload_${PL})
endforeach(PL)