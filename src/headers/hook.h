#ifndef HOOK_H
#define HOOK_H

#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include "utils.h"

void patch_rax_reg(struct pt_regs *regs);
int hook_syscall_table(void);

#endif
