#ifndef DR_B_H
#define DR_B_H

#include "utils.h"

#include <linux/smp.h>
#include <asm/desc.h>

typedef void (*bp_handler)(struct pt_regs *regs);

/* dr breakpoint struct */
typedef struct dr_bp_t {
  unsigned long dr[4];
  unsigned long dr6, dr7;
  bp_handler handlers[4];
} dr_bp;

enum bp_type {
  BP_X = 0,
  BP_RW,
};

/* macros for DR6 */
#define DR_TRAP0 (1 << 0)
#define DR_TRAP1 (1 << 1)
#define DR_TRAP2 (1 << 2)
#define DR_TRAP3 (1 << 3)
#define DR_BD	 (1 << 13)
#define DR_BS	 (1 << 14)
#define DR_BT	 (1 << 15)

/* macros for DR7 */
#define DR_LE (1 << 8)
#define DR_GE (1 << 9)
#define DR_RE (1 << 10)
#define DR_RT (1 << 11)
#define DR_GD (1 << 13)

/* general DR macros */
#define DR_RW_EXECUTE 0x0
#define DR_RW_WRITE   0x1
#define DR_RW_READ    0x3

#define DR_LEN_1 0x0
#define DR_LEN_2 0x1
#define DR_LEN_4 0x3
#define DR_LEN_8 0x2

#define __set_dr(num, val) \
	asm volatile ("mov %0,%%db" #num : : "r" (val))
#define __get_dr(num, val) \
        asm volatile("mov %%db" #num ",%0" : "=r" (val))

#define DO_DEBUG_VECTOR 1
#define HML_TO_ADDR(h,m,l)							 \
((unsigned long) (l) | ((unsigned long) (m) << 16) | \
((unsigned long) (h) << 32))

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

static inline void get_dr(unsigned char num, unsigned long *val)
{
  switch (num) {
  case 0:
	__get_dr(0, *val);
	break;
  case 1:
	__get_dr(1, *val);
	break;
  case 2:
	__get_dr(2, *val);
	break;
  case 3:
	__get_dr(3, *val);
	break;
  case 6:
	__get_dr(6, *val);
	break;
  case 7:
	__get_dr(7, *val);
	break;
  }
}

static inline void set_dr(unsigned char num, unsigned long val)
{
  switch (num) {
  case 0:
	__set_dr(0, val);
	break;
  case 1:
	__set_dr(1, val);
	break;
  case 2:
	__set_dr(2, val);
	break;
  case 3:
	__set_dr(3, val);
	break;
  case 6:
	__set_dr(6, val);
	break;
  case 7:
	__set_dr(7, val);
	break;
  }
}

struct __drreg {
  unsigned char num;
  unsigned long val;
};

static void __on_each_cpu_set_dr(void *data)
{
  struct __drreg *dr = data;
  set_dr(dr->num, dr->val);
}

static inline void on_each_cpu_set_dr(unsigned char num, unsigned long val)
{
  struct __drreg dr = {
	.num = num,
	.val = val,
  };

  on_each_cpu(__on_each_cpu_set_dr, &dr, 0);
}

extern int is_root;

int reg_dr_bp(unsigned long addr, int type, int len, bp_handler handler);
int patch_idt(void);
void unpatch_idt(void);

#endif
