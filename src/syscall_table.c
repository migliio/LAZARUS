#include "utils.h"
#include "syscall_table.h"

unsigned long *sys_call_table_retrieve(void)
{
  return (unsigned long *)kallsyms_lookup_name("sys_call_table");
}

/* make SCT writeable */
int set_sct_rw(unsigned long *table_ptr)
{
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long)table_ptr, &level);
  if (pte->pte &~_PAGE_RW) {
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

/* make SCT read only */
int set_sct_ro(unsigned long *table_ptr)
{
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long)table_ptr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

/* get the system call handler from MSR */
unsigned long get_64_sys_call_handler(void)
{
  unsigned long __sys_handler_entry;
  rdmsrl(MSR_LSTAR, __sys_handler_entry);
  return __sys_handler_entry;
}

/* CALL rel32 - relative displacement */
unsigned long get_syscall_64_addr(void)
{
  int i;
  unsigned long addr = get_64_sys_call_handler(); // get the system call handler
  unsigned char *op = (unsigned char *)addr;
  for (i = 0; i < 512; i++) {
	if (is_call(op)) {
	  addr = get_call_off(op);
	  return addr;
	}
	op++;
  }
  return (unsigned long)NULL;
}

/* sbb %rdx,%rdx is the next instruction after %rax check */
unsigned long get_gadget_addr(unsigned long call_sys_addr)
{
  int i;
  unsigned char *op = (unsigned char*)call_sys_addr;
  for (i = 0; i < 512; i++) {
	if (is_and_in(op))
	  return (unsigned long)op;
	op++;
  }
  return (unsigned long)NULL;
}
