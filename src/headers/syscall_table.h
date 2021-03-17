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
void *get_syscall_64_addr(void);

static inline int is_call_syscall(unsigned char *op)
{
  if (op[0] == 0xe8) {
	debug_print("Call found in syscall dispatcher");
	return 1;
  }

  return 0;
}

static inline unsigned long get_do_sys_displacement(unsigned char *op)
{
  unsigned char *temp = kmalloc(sizeof(char)*4, GFP_KERNEL);
  long off;
  int err;

  if (!temp)
	return -ENOBUFS;
  
  memcpy(temp, &op[1], sizeof(char)*4);

  debug_print("Hex value found: %04x", temp);

  return (unsigned long)temp;
}

#endif
