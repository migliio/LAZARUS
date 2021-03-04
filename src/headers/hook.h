#ifndef HOOK_H
#define HOOK_H

#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include "utils.h"

void system_call_hook(struct pt_regs *regs);
int hook_sys_call_table(void);
int register_dr_breakpoint(void);
int unregister_dr_breakpoint(void);

#endif
