project(checkm8_libpayload)

set(PL_NAMES_SHORT
        aes
        aes_busy
        aes_sw
        sync
        sysreg
        task_sleep_test)

foreach(NAME ${PL_NAMES_SHORT})
    list(APPEND PL_TARGETS      "payload_${NAME}")
    list(APPEND PL_SRC_SHORT    "${CMAKE_CURRENT_LIST_DIR}/pl/src/${NAME}.c")
endforeach(NAME)

foreach(NAME ${PL_TARGETS})
    list(APPEND PL_SRC_LONG     "${CMAKE_CURRENT_BINARY_DIR}/lib/${NAME}.c")
    list(APPEND PL_BIN          "${CMAKE_CURRENT_BINARY_DIR}/pl/bin/${NAME}.bin")
    list(APPEND PL_TARGETS_BIN  ${NAME}_bin)
endforeach(NAME)

add_subdirectory(pl)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_custom_target(payload_sources
                    BYPRODUCTS  ${PL_SRC_LONG}
                    DEPENDS     ${PL_TARGETS_BIN}
                    COMMENT     "Refreshing payload library"
                    COMMAND     python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/librarize.py
                                ${CMAKE_CURRENT_BINARY_DIR}/pl/bin
                                ${CMAKE_CURRENT_BINARY_DIR}/lib)

add_library(payload ${PL_SRC_LONG})
add_dependencies(payload payload_sources)

add_custom_command(TARGET       payload POST_BUILD
                    BYPRODUCTS  ${CMAKE_SOURCE_DIR}/c8_remote/include/libpayload.h
                    COMMAND     python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/headerize.py
                                        ${CMAKE_CURRENT_BINARY_DIR}/lib
                                        ${CMAKE_SOURCE_DIR}/c8_remote/include)