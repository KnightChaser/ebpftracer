// src/syscall_table.c
#include "syscall_table.h"
#include "syscalls/open_common.h"
#include "syscalls/syscalls.h"
#include <stddef.h>

syscall_handler_t enter_handlers[MAX_SYSCALL_NR];
syscall_handler_t exit_handlers[MAX_SYSCALL_NR];

/**
 * Initializes the syscall handlers to default handlers.
 * This function sets up the enter and exit handlers for syscalls.
 * It can be extended to register specific syscall handlers.
 */
void syscall_table_init(void) {
    // NOTE: Initialize all syscall handlers to default handlers
    for (size_t i = 0; i < MAX_SYSCALL_NR; i++) {
        enter_handlers[i] = handle_sys_enter_default;
        exit_handlers[i] = handle_sys_exit_default;
    }

    // NOTE: Register specific syscall handlers

    // open/openat/openat2 syscalls
    REGISTER_SYSCALL_HANDLER(SYS_open, open_enter_dispatch, open_exit_dispatch);
    REGISTER_SYSCALL_HANDLER(SYS_openat, open_enter_dispatch,
                             open_exit_dispatch);
#ifdef SYS_openat2
    if (IS_SYSCALL_SUPPORTED(SYS_openat2)) {
        REGISTER_SYSCALL_HANDLER(SYS_openat2, open_enter_dispatch,
                                 open_exit_dispatch);
    }
#endif

    // close(2)
    REGISTER_SYSCALL_HANDLER(SYS_close, handle_close_enter, handle_close_exit);

    // dup/dup2/dup3
    REGISTER_SYSCALL_HANDLER(SYS_dup, handle_dup_enter, handle_dup_exit);
    REGISTER_SYSCALL_HANDLER(SYS_dup2, handle_dup2_enter, handle_dup2_exit);
#ifdef SYS_dup3
    if (IS_SYSCALL_SUPPORTED(SYS_dup3)) {
        REGISTER_SYSCALL_HANDLER(SYS_dup3, handle_dup3_enter, handle_dup3_exit);
    }

    // fcntl
    REGISTER_SYSCALL_HANDLER(SYS_fcntl, handle_fcntl_enter, handle_fcntl_exit);

#endif
}
