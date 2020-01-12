project(checkm8_libpayload)

set(PL_NAMES_SHORT
        aes
        aes_busy
        aes_sw
        sync
        sysreg
        task_sleep_test)

foreach(NAME ${PL_NAMES_SHORT})
    list(APPEND PL_TARGETS "payload_${NAME}")
    list(APPEND PL_SRC_SHORT  "${CMAKE_CURRENT_LIST_DIR}/pl/src/${NAME}.c")
endforeach(NAME)

foreach(NAME ${PL_TARGETS})
    list(APPEND PL_SRC_LONG "${CMAKE_CURRENT_BINARY_DIR}/lib/${NAME}.c")
endforeach(NAME)

add_subdirectory(pl)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
add_custom_target(libpayload_sources        # TODO: somehow only pass names that need to be udpated?
                    DEPENDS ${PL_TARGETS}
                    BYPRODUCTS ${PL_SRC_LONG}
                    COMMENT "running librarizer"
                    COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/scripts/librarize.py
                                    ${CMAKE_CURRENT_BINARY_DIR}/pl/bin
                                    ${CMAKE_CURRENT_BINARY_DIR}/lib)

add_library(payload ${PL_SRC_LONG})
add_dependencies(payload libpayload_sources)