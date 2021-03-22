#include "dr_breakpoints.h"
#include "syscall_table.h"
#include "utils.h"

dr_bp bp;

unsigned int do_debug_v = DO_DEBUG_VECTOR;
gate_desc old_gate_entry, new_gate_desc;

static unsigned int old_rip_off;
static unsigned int *patched_addr = NULL;

static void emulate_cpu(struct pt_regs *regs)
{
	regs->ip += 3;
}

asmlinkage void my_do_debug(struct pt_regs *regs, long error_code)
{
	unsigned long dr6;
	bp_handler handler;

	debug_print("do_debug hijacked");

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
  gate_desc *gate_ptr;
  unsigned char *ptr;
  unsigned int rip_offset;
  unsigned long addr;
  int i, call_found = 0;
  
  store_idt(&idtr);

  gate_ptr = (gate_desc *) (idtr.address + do_debug_v * sizeof(gate_desc));
  addr = (unsigned long) HML_TO_ADDR(gate_ptr->offset_high, gate_ptr->offset_middle, gate_ptr->offset_low);
  
  ptr = (unsigned char *)addr;
  for (i = 0; i < 512; i++) {
	if (is_call(ptr)) {
	  if (call_found == 1) {
		patched_addr = (unsigned int *)ptr;
		old_rip_off = *patched_addr;
		rip_offset = (unsigned long)my_do_debug - (unsigned long)patched_addr - 4;

		clear_CR0_WP();
		*patched_addr = rip_offset;
		set_CR0_WP();
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
	clear_CR0_WP();
	*patched_addr = old_rip_off;
	set_CR0_WP();
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

  on_each_cpu_set_dr(i, bp.dr[i]);
  on_each_cpu_set_dr(7, bp.dr[7]);

  debug_print("DR breakpoint set in DR%d at address %p", i, (void *)bp.dr[i]);

  return 0;
}
