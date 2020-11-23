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
