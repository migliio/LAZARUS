#include "dr_breakpoints.h"
#include "syscall_table.h"
#include "utils.h"

dr_bp bp;

unsigned int do_debug_v = DO_DEBUG_VECTOR;
gate_desc old_gate_desc, new_gate_desc;

static unsigned int old_rip_off = 0x0;
static unsigned int new_rip_off = 0x0;
static unsigned int *patched_addr = NULL;

static void emulate_cpu(struct pt_regs *regs)
{
	regs->ip += 3;
}

asmlinkage void my_do_debug(struct pt_regs *regs, long error_code)
{
	unsigned long dr6;
	bp_handler handler;

	get_dr(6, &dr6);

	if (dr6 & DR_BD) {
		dr6 &= ~DR_BD;
		emulate_cpu(regs);
	}
	if (dr6 & DR_TRAP0) {
		dr6 &= ~DR_TRAP0;
		handler = bp.handlers[0];
		goto trap;
	} else if (dr6 & DR_TRAP1) {
		dr6 &= ~DR_TRAP1;
		handler = bp.handlers[1];
		goto trap;
	} else if (dr6 & DR_TRAP2) {
		dr6 &= ~DR_TRAP2;
		handler = bp.handlers[2];
		goto trap;
	} else if (dr6 & DR_TRAP3) {
		dr6 &= ~DR_TRAP3;
		handler = bp.handlers[3];
		goto trap;
	}
	return;
trap:
	regs->flags |= X86_EFLAGS_RF;
	regs->flags &= ~X86_EFLAGS_TF;
	handler(regs);
}

int patch_idt(void)
{
  struct desc_ptr idtr;
  unsigned char *ptr;
  long addr;
  gate_desc *gate_ptr;
  int i, call_found = 0;
  unsigned int rip_off;
  
  store_idt(&idtr);

  gate_ptr = (gate_desc *) (idtr.address + do_debug_v * sizeof(gate_desc));
  addr = (unsigned long) HML_TO_ADDR(gate_ptr->offset_high, gate_ptr->offset_middle, gate_ptr->offset_low);
  if (!addr)
	return -ENODATA;
  memcpy(&old_gate_desc, (void *) (idtr.address + do_debug_v * sizeof(gate_desc)), sizeof(gate_desc));
  pack_gate(&new_gate_desc, GATE_TRAP, (unsigned long)addr, 0x3, 0, 0);
  
  ptr = (unsigned char *)addr;
  for (i = 0; i < 512; i++) {
	if (is_call(ptr)) {
	  if (call_found == 1) {
		rip_off = ((unsigned int) ptr[1]) | (((unsigned int) ptr[2]) << 8)
		  | (((unsigned int) ptr[3]) << 16) | (((unsigned int) ptr[4]) << 24);
		old_rip_off = rip_off;

		new_rip_off = (unsigned int) ((long) my_do_debug - (long) &ptr[5]);
		patched_addr = (unsigned int *) &ptr[1];

		clear_CR0_WP();
		arch_cmpxchg(patched_addr, old_rip_off, new_rip_off);
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
  arch_cmpxchg(patched_addr, new_rip_off, old_rip_off);
  write_idt_entry((gate_desc *)idtr.address, do_debug_v, &old_gate_desc);
  set_CR0_WP();
  
  debug_print("IDT unpatching done");
}

int reg_dr_bp(unsigned long addr, int type, int len, bp_handler handler)
{
  int i;
  static int init = 0;
  unsigned long dr7;

  if (!init) {
	memset(&bp, 0, sizeof(bp));
	init = 1;
  }

  for (i = 0; i < 4; i++)
	if (!bp.dr[i])
	  break;

  if (i == 4)
	return -1;
	
  bp.dr[i] = addr;
  bp.handlers[i] = handler;
  
  dr7 = (len | type) & 0xf;
  dr7 <<= (16 + i * 4);
  dr7 |= 0x2 << (i * 2);

  // bp.dr7 |= DR_GD;
  bp.dr7 |= dr7 | DR_LE | DR_GE;
  bp.dr7 ^= (DR_RE | DR_RT);

  set_dr_on_each_cpu(i, bp.dr[i]);
  set_dr_on_each_cpu(7, bp.dr7);

  debug_print("DR breakpoint set in DR%d at address %p", i, (void *)bp.dr[i]);
  
  return 0;
}
