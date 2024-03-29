#include "utils.h"
#include "syscall_table.h"

/* system call table address */
unsigned long *table_ptr = NULL;

/* set the system call table address */
int set_syscall_table(void) {
  
  table_ptr = sys_call_table_retrieve();

  if (table_ptr == NULL)
    return 1;
  
  return 0;
}
