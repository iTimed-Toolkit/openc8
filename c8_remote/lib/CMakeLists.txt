set(PL_NAMES
        exploit:checkm8
        exploit:handler
        exploit:checkra1n_stage1
        exploit:checkra1n_stage2
        exploit:checkra1n_handler
        exploit:patch_tables

        crypto:aes_sw_bern
        crypto:aes_hw
        crypto:aes_hw_builtin

        system:cachelib
        system:exit_usb_task
        system:gpio_test
        system:sync

        power:wfi_test

        util:read
        util:write
        util:exec
        util:data
        util:async
        )

foreach(NAME ${PL_NAMES})
    string(REGEX REPLACE "^[^:]*\\:(.*)$" "\\1" NAME_ONLY ${NAME})
    list(APPEND PL_TARGETS      "payload_${NAME_ONLY}")
endforeach(NAME)

foreach(TARGET ${PL_TARGETS})
    list(APPEND PL_SRC_LIB      "${CMAKE_CURRENT_BINARY_DIR}/payload/c/${TARGET}.c")
    list(APPEND PL_BIN          "${CMAKE_CURRENT_BINARY_DIR}/payload/bin/${TARGET}.bin")
endforeach(TARGET)

set(CMAKE_C_FLAGS "-g -Wall")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/payload)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/payload/c)

list(APPEND TO_LIBRARIZE "${CMAKE_CURRENT_BINARY_DIR}/payload/bin/")

if(WITH_PONGO)
    list(APPEND PL_TARGETS payload_pongo)
    list(APPEND PL_SRC_LIB "${CMAKE_CURRENT_BINARY_DIR}/payload/c/payload_pongo.c")
    list(APPEND TO_LIBRARIZE ${CMAKE_CURRENT_BINARY_DIR}/payload/pongoOS/Pongo.bin)
endif()

add_custom_target(payload_sources
                    BYPRODUCTS  ${PL_SRC_LIB}
                                ${CMAKE_CURRENT_BINARY_DIR}/payload/c/get_payloads.c
                                ${CMAKE_CURRENT_BINARY_DIR}/include/libpayload.h
                                ${CMAKE_CURRENT_BINARY_DIR}/include/libpayload_int.h
                    DEPENDS     ${PL_TARGETS}
                    COMMENT     "Refreshing payload library"
                    COMMAND     python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/librarize.py
                                        ${TO_LIBRARIZE}
                                        ${CMAKE_CURRENT_BINARY_DIR}/payload)
add_library(payload ${PL_SRC_LIB}
                ${CMAKE_CURRENT_BINARY_DIR}/payload/c/get_payloads.c)
add_dependencies(payload payload_sources)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/experiments)
add_library(host_crypto crypto/aes.c)