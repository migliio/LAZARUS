#ifndef HOOK_H
#define HOOK_H

#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <asm/desc.h>

#define DO_DEBUG_VECTOR 3
#define HML_TO_ADDR(h,m,l)							 \
((unsigned long) (l) | ((unsigned long) (m) << 16) | \
((unsigned long) (h) << 32))

extern int is_root;

int patch_idt(void);
int hook_syscall_table(void);

#endif
