set(PL_NAMES
        crypto:aes_busy
        crypto:aes_sw_bern
        crypto:aes_sw_corr

        system:cachelib
        system:exit_usb_task
        system:sync

        power:floppysleep
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

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/payload/c)
add_custom_target(payload_sources
                    BYPRODUCTS  ${PL_SRC_LIB}
                    DEPENDS     ${PL_TARGETS}
                    COMMENT     "Refreshing payload library"
                    COMMAND     python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/librarize.py
                                        ${CMAKE_CURRENT_BINARY_DIR}/payload/bin
                                        ${CMAKE_CURRENT_BINARY_DIR}/payload/c)

add_library(payload ${PL_SRC_LIB})
add_dependencies(payload payload_sources)

add_custom_command(TARGET       payload POST_BUILD
                    BYPRODUCTS  ${CMAKE_SOURCE_DIR}/c8_remote/include/libpayload.h
                    COMMAND     python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/headerize.py
                                        ${CMAKE_CURRENT_BINARY_DIR}/payload/c
                                        ${CMAKE_SOURCE_DIR}/c8_remote/include/tool)

add_library(experiments experiments/crypto.c
                        experiments/system.c
                        experiments/power.c)
add_library(host_crypto crypto/aes_sw_impl.c crypto/aes_sw_host.c)