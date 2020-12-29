include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER    /opt/aarch64-none-elf/bin/aarch64-none-elf-gcc)
set(CMAKE_ASM_COMPILER  /opt/aarch64-none-elf/bin/aarch64-none-elf-gcc)
set(CMAKE_OBJCOPY       /opt/aarch64-none-elf/bin/aarch64-none-elf-objcopy)
set(CMAKE_RANLIB        /opt/aarch64-none-elf/bin/aarch64-none-elf-ranlib)

set(CMAKE_C_FLAGS "-nostdlib -Os -Wl,--gc-sections -fno-ipa-cp -fno-builtin")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

foreach(NAME ${PL_NAMES})
    string(REPLACE ":" "/" SRC_PATH ${NAME})
    string(REGEX REPLACE "^[^:]*\\:(.*)$" "\\1" NAME_ONLY ${NAME})

    set(SRC_NAME ${NAME}_SRCS)
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
                        ARGS        -O binary -j .text -j .payload_text* -j .payload_data_*
                                    ${CMAKE_CURRENT_BINARY_DIR}/payload_${NAME_ONLY}
                                    ${CMAKE_CURRENT_BINARY_DIR}/bin/payload_${NAME_ONLY}.bin)
endforeach(NAME)

add_library(dev_crypto ../crypto/aes.c)
target_compile_definitions(dev_crypto PRIVATE DEV_CRYPTO)

target_link_libraries(payload_aes_sw_bern dev_crypto)

if(WITH_PONGO)
    add_subdirectory(pongoOS)
endif()