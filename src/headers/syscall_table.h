#ifndef SYSCALL_H
#define SYSCALL_H

#include <asm/desc.h>
#include <asm/desc_defs.h>
#include <asm/msr.h>
#include <asm/msr-index.h>

#include "utils.h"

#define load_addr_from_cs(ptr,var) asm volatile("mov %%cs:(%0), %%rax":"=a" (var):"a" (ptr))

void *sys_call_table_retrieve(void);

int set_sct_rw(unsigned long table_ptr);
int set_sct_ro(unsigned long table_ptr);
u8 *get_64_sys_call_handler(void);
unsigned long get_syscall_64_addr(void);
unsigned long get_gadget_addr(void *call_sys_addr);

static inline int is_call_syscall(unsigned char *op)
{
  if (op[0] == 0xe8) {
	debug_print("Call found in syscall dispatcher");
	return 1;
  }

  return 0;
}

/* 
 * "CALL rel32" means that rel32 needs to be sign-extended to 64 bits
 */
static inline unsigned long get_do_sys_off(unsigned char *op)
{
  unsigned long off;
  int32_t rel32 = *(int32_t *)(op + 1);

  off = (unsigned long)(op + 5 + rel32);
  return off;
}

static inline int is_sbb_in(unsigned char *op)
{
  if (op[0] == 0x48 && op[1] == 0x19) {
		debug_print("Gadget found in do_syscall_64");
		return 1;
  }

  return 0;
}

#endif
