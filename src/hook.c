#include "syscall_table.h"
#include "hook.h"

#define SYSCALL(sys, args...) ((long(*)())sct[__NR_##sys])(args)
#define HOOK(sys, func) fake_sct[__NR_##sys] = (unsigned long)func

static unsigned long fake_sct[320];
static unsigned long *sct;
unsigned long do_syscall_call;

struct perf_event *__percpu *sample_hbp;

/*
 * rax -> fake_sct index
 */
void system_call_hook(struct pt_regs *regs)
{
  unsigned long off = ((unsigned long)fake_sct - (unsigned long)sct);
  regs->ax += off / sizeof(unsigned long);
}

int hook_sys_call_table(void)
{
  sct = (unsigned long *)table_ptr;
  if (!sct) {
	return -1;
  }

  memcpy(fake_sct, sct, sizeof(fake_sct));

  return 0;
}

static void sample_hbp_handler(struct perf_event *bp,
							   struct perf_sample_data *data,
							   struct pt_regs *regs)
{
  // hook_sys_call_table();
  // system_call_hook(regs);
  debug_print("Hardware breakpoint handler executed\n");
}

int register_dr_breakpoint(void)
{
  int ret;
  struct perf_event_attr attr; // performance event attributes
  unsigned long gd_addr;
  unsigned long call_sys_addr = get_syscall_64_addr();

  if (!call_sys_addr)
	return -ENXIO; // set errno to "no such device or address"

  debug_print("do_syscall_64 is at address %p", (void *)call_sys_addr);

  gd_addr = get_gadget_addr((void *)call_sys_addr);

  if (!gd_addr)
	return -ENXIO;

  hw_breakpoint_init(&attr); // create the performance event to monitor
  attr.bp_addr = gd_addr;
  attr.bp_len = sizeof(long);
  attr.bp_type = HW_BREAKPOINT_X; // set an execution breakpoint

  sample_hbp = register_wide_hw_breakpoint(&attr, sample_hbp_handler, NULL);
  if (IS_ERR((void __force *)sample_hbp)) {
	ret = PTR_ERR((void __force *)sample_hbp);
	goto fail;
  }

  debug_print("HW Breakpoint installed");

  return 0;

 fail:
  printk(KERN_INFO "Breakpoint registration failed");
  
  return ret;
}
