project(checkm8_libpayload_sources C ASM)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)


set(CMAKE_SYSTEM_PROCESSOR arm)
if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    # regular desktop
    set(CMAKE_C_COMPILER    /usr/bin/aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER  /usr/bin/aarch64-linux-gnu-as)
    set(CMAKE_OBJCOPY       /usr/bin/aarch64-linux-gnu-objcopy)
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "armv7l")
    # raspberry pi
    set(CMAKE_C_COMPILER    /opt/cross/bin/aarch64-linux-gcc)
    set(CMAKE_ASM_COMPILER  /opt/cross/bin/aarch64-linux-as)
    set(CMAKE_OBJCOPY       /opt/cross/bin/aarch64-linux-objcopy)
endif()

set(CMAKE_C_FLAGS "-nostdlib -O -Wl,--gc-sections")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

foreach(NAME ${PL_NAMES})
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/src/${NAME}.S)
        add_executable(payload_${NAME} ${CMAKE_CURRENT_LIST_DIR}/payload_entry.c
                                       ${CMAKE_CURRENT_LIST_DIR}/src/${NAME}.c
                                       ${CMAKE_CURRENT_LIST_DIR}/src/${NAME}.S)
    else()
        add_executable(payload_${NAME} ${CMAKE_CURRENT_LIST_DIR}/payload_entry.c
                                       ${CMAKE_CURRENT_LIST_DIR}/src/${NAME}.c)
    endif()

    add_custom_command(TARGET       payload_${NAME} POST_BUILD
                        BYPRODUCTS  ${CMAKE_CURRENT_BINARY_DIR}/bin/payload_${NAME}.bin
                        COMMAND     ${CMAKE_OBJCOPY}
                        ARGS        -O binary -j .text -j .payload_text
                                    ${CMAKE_CURRENT_BINARY_DIR}/payload_${NAME}
                                    ${CMAKE_CURRENT_BINARY_DIR}/bin/payload_${NAME}.bin)
endforeach(NAME)