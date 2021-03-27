#ifndef SYSCALL_H
#define SYSCALL_H

#include <asm/desc.h>
#include <asm/desc_defs.h>
#include <asm/msr.h>
#include <asm/msr-index.h>
#include <linux/syscalls.h>

#include "utils.h"

unsigned long *sys_call_table_retrieve(void);

int set_sct_rw(unsigned long *table_ptr);
int set_sct_ro(unsigned long *table_ptr);

unsigned long get_syscall_64_addr(void);
unsigned long get_gadget_addr(unsigned long call_sys_addr);

/* call opcode */
static inline int is_call(unsigned char *op)
{
  if (op[0] == 0xe8)
	return 1;
  return 0;
}

/* and opcode */
static inline int is_and_in(unsigned char *op)
{
  if (op[0] == 0x48 && op[1] == 0x21)
		return 1;
  return 0;
}

/*
 * "CALL rel32" means that rel32 needs to be sign-extended to 64 bits
 */
static inline unsigned long get_call_off(unsigned char *op)
{
  unsigned long off;
  int32_t rel32 = *(int32_t *)(op + 1);

  off = (unsigned long)(op + 5 + rel32);
  return off;
}

#endif
