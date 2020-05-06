include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    # regular desktop
    set(CMAKE_C_COMPILER    /usr/bin/aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER  /usr/bin/aarch64-linux-gnu-as)
    set(CMAKE_OBJCOPY       /usr/bin/aarch64-linux-gnu-objcopy)
    set(CMAKE_RANLIB        /usr/bin/aarch64-linux-gnu-ranlib)
elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "armv7l")
    # raspberry pi
    set(CMAKE_C_COMPILER    /opt/cross/bin/aarch64-linux-gcc)
    set(CMAKE_ASM_COMPILER  /opt/cross/bin/aarch64-linux-as)
    set(CMAKE_OBJCOPY       /opt/cross/bin/aarch64-linux-objcopy)
    set(CMAKE_RANLIB        /opt/cross/bin/aarch64-linux-ranlib)
endif()

set(CMAKE_C_FLAGS "-nostdlib -O")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

foreach(NAME ${PL_NAMES})
    string(REPLACE ":" "/" SRC_PATH ${NAME})
    string(REGEX REPLACE "^[^:]*\\:(.*)$" "\\1" NAME_ONLY ${NAME})

    set(SRC_NAME ${NAME}_SRCS)
    set(${SRC_NAME} src/payload_entry.S src/payload_entry.c)

    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/src/${SRC_PATH}.c)
        list(APPEND ${SRC_NAME} src/${SRC_PATH}.c)
    endif()

    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/src/${SRC_PATH}.S)
        list(APPEND ${SRC_NAME} src/${SRC_PATH}.S)
    endif()

    add_executable(payload_${NAME_ONLY} ${${SRC_NAME}})
    add_custom_command(TARGET       payload_${NAME_ONLY} POST_BUILD
                        BYPRODUCTS  ${CMAKE_CURRENT_BINARY_DIR}/bin/payload_${NAME_ONLY}.bin
                        COMMAND     ${CMAKE_OBJCOPY}
                        ARGS        -O binary -j .text -j .payload_text -j .payload_data
                                    ${CMAKE_CURRENT_BINARY_DIR}/payload_${NAME_ONLY}
                                    ${CMAKE_CURRENT_BINARY_DIR}/bin/payload_${NAME_ONLY}.bin)
endforeach(NAME)

add_library(dev_crypto ../crypto/aes_sw_impl.c)
target_compile_definitions(dev_crypto PRIVATE DEV_CRYPTO)

target_link_libraries(payload_aes_sw_bern dev_crypto)