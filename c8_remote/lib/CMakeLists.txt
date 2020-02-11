project(checkm8_libpayload)

set(PL_NAMES
        aes_busy
        aes_sw
        exit_usb_task
        floppysleep
        sync
        )

foreach(NAME ${PL_NAMES})
    list(APPEND PL_TARGETS      "payload_${NAME}")
    list(APPEND PL_SRC_BIN      "${CMAKE_CURRENT_LIST_DIR}/payload/src/${NAME}.c")
endforeach(NAME)

foreach(TARGET ${PL_TARGETS})
    list(APPEND PL_SRC_LIB      "${CMAKE_CURRENT_BINARY_DIR}/lib_cfiles/${TARGET}.c")
    list(APPEND PL_BIN          "${CMAKE_CURRENT_BINARY_DIR}/payload/bin/${TARGET}.bin")
endforeach(TARGET)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/payload)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib_cfiles)
add_custom_target(payload_sources
                    BYPRODUCTS  ${PL_SRC_LIB}
                    DEPENDS     ${PL_TARGETS}
                    COMMENT     "Refreshing payload library"
                    COMMAND     python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/librarize.py
                                        ${CMAKE_CURRENT_BINARY_DIR}/payload/bin
                                        ${CMAKE_CURRENT_BINARY_DIR}/lib_cfiles)

add_library(payload ${PL_SRC_LIB})
add_dependencies(payload payload_sources)

add_custom_command(TARGET       payload POST_BUILD
                    BYPRODUCTS  ${CMAKE_SOURCE_DIR}/c8_remote/include/libpayload.h
                    COMMAND     python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/headerize.py
                                        ${CMAKE_CURRENT_BINARY_DIR}/lib_cfiles
                                        ${CMAKE_SOURCE_DIR}/c8_remote/include)