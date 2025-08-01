// src/syscalls/handlers/handle_read.h
#pragma once
#include "../../controller.h"
#include "../read_common.h"
#include <sys/types.h>

void handle_read_enter(pid_t pid, const struct syscall_event *e,
                       const struct read_args *args);
void handle_read_exit(pid_t pid, const struct syscall_event *e);
