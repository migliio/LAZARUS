#include "utils.h"
#include "syscall_table.h"

void *sys_call_table_retrieve(void)
{
  /* looking for the system call table address in exported symbols */
  return (void *)kallsyms_lookup_name("sys_call_table");
}

/* make SCT writeable */
int set_sct_rw(unsigned long table_ptr)
{
  unsigned int level;
  pte_t *pte = lookup_address(table_ptr, &level);
  if (pte->pte &~_PAGE_RW) {
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

/* make SCT read only */
int set_sct_ro(unsigned long table_ptr)
{
  unsigned int level;
  pte_t *pte = lookup_address(table_ptr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

/* get the system call handler from MSR */
u8 *get_64_sys_call_handler(void)
{
  u64 __sys_handler_entry;
  rdmsrl(MSR_LSTAR, __sys_handler_entry);
  return (u8 *) __sys_handler_entry;
}

/* CALL rel32 - relative displacement */
void *get_syscall_64_addr(void)
{
  int i;
  unsigned long off, cs_off;
  void *addr = (void *) get_64_sys_call_handler(); // get the system call handler
  unsigned char *op = (unsigned char *)addr;
  for (i = 0; i < 512; i++) {
	if (is_call_syscall(op)) {
	  debug_print("Call to do_syscall_64 is at address %p", (void *)op);
	  off = get_do_sys_displacement(op);
	  if (!off)
		return NULL;	  
	  load_addr_from_cs(&off, cs_off);
	  return (void *)cs_off;
	}
	op++;
  }
  return NULL;
}
