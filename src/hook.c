#include "syscall_table.h"
#include "hook.h"

#define SYSCALL(sys, args...) ((long(*)())sct[__NR_##sys])(args)
#define HOOK(sys, func) fake_sct[__NR_##sys] = (unsigned long)func

static unsigned long fake_sct[320];
static unsigned long *sct;

struct perf_event *__percpu *sample_hbp;
static char ksym_name[KSYM_NAME_LEN] = "syscall_return_slowpath";

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
}

static void sample_hbp_handler(struct perf_event *bp,
							   struct perf_sample_data *data,
							   struct pt_regs *regs)
{
  system_call_hook(regs);
  printk(KERN_INFO "%s executed\n", ksym_name);
}

int register_dr_breakpoint(void)
{
  int ret;
  struct perf_event_attr attr; // performance event attributes
  void *addr = __symbol_get(ksym_name); // get the symbol address

  printk(KERN_INFO "HW Breakpoint is at address %p", addr);

  if (!addr)
	return -ENXIO; // set errno to "no such device or address"

  hw_breakpoint_init(&attr); // create the performance event to monitor
  attr.bp_addr = (unsigned long)addr;
  attr.bp_len = sizeof(long);
  attr.bp_type = HW_BREAKPOINT_X; // set an execution breakpoint

  sample_hbp = register_wide_hw_breakpoint(&attr, sample_hbp_handler, NULL);
  if (IS_ERR((void __force *)sample_hbp)) {
	ret = PTR_ERR((void __force *)sample_hbp);
	goto fail;
  }

  printk(KERN_INFO "HW Breakpoint for %s installed\n", ksym_name);

  return 0;

 fail:
  printk(KERN_INFO "Breakpoint registration failed\n");
  
  return ret;
}
