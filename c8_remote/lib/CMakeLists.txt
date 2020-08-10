set(PL_NAMES
        exploit:checkm8
        exploit:handler
        exploit:patch_tables

        crypto:aes_sw_bern
        crypto:aes_hw

        system:cachelib
        system:exit_usb_task
        system:sync

        power:floppysleep
        power:trigger

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
add_custom_target(payload_sources
                    BYPRODUCTS  ${PL_SRC_LIB}
                                ${CMAKE_CURRENT_BINARY_DIR}/payload/c/payload_pongo.c
                                ${CMAKE_CURRENT_BINARY_DIR}/payload/c/get_payloads.c
                                ${CMAKE_CURRENT_BINARY_DIR}/include/libpayload.h
                                ${CMAKE_CURRENT_BINARY_DIR}/include/libpayload_int.h
                    DEPENDS     ${PL_TARGETS} payload_pongo
                    COMMENT     "Refreshing payload library"
                    COMMAND     python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/librarize.py
                                        ${CMAKE_CURRENT_BINARY_DIR}/payload/bin
                                        ${CMAKE_CURRENT_BINARY_DIR}/payload)

add_library(payload ${PL_SRC_LIB}
                ${CMAKE_CURRENT_BINARY_DIR}/payload/c/payload_pongo.c
                ${CMAKE_CURRENT_BINARY_DIR}/payload/c/get_payloads.c)
add_dependencies(payload payload_sources)

add_library(experiments experiments/crypto.c
                        experiments/system.c
                        experiments/power.c)
add_library(host_crypto crypto/aes.c)