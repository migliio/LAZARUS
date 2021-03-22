#ifndef HOOK_H
#define HOOK_H

#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <asm/desc.h>

#define DO_DEBUG_VECTOR 1
#define HML_TO_ADDR(h,m,l)							 \
((unsigned long) (l) | ((unsigned long) (m) << 16) | \
((unsigned long) (h) << 32))

extern int is_root;

static inline void set_CR0_WP(void)
{
	unsigned long cr0;

	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x00010000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));
}

static inline void clear_CR0_WP(void)
{
	unsigned long cr0;

	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 &= ~0x00010000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));
}

int patch_idt(void);
void unpatch_idt(void);
int hook_syscall_table(void);

#endif
