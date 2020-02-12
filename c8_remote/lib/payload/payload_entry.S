.extern entry_sync
.extern entry_async
.extern load_sync_entry

.global _start
.section .text
_start:
    mov     x10,        x30
    bl      load_sync_entry
    mov     x30,        x10

    # if we came from the synchronous entry point, branch to entry_sync
    cmp     x9,         x10
    b.eq    entry_sync

    # else branch to the payload's async entry points
    b       entry_async
