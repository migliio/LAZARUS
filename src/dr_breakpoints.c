#include "dr_breakpoints.h"
#include "utils.h"

int reg_dr_breakpoint(unsigned long addr, int type, int len)
{
  int i;
  static int init = 0;
  unsigned long __dr7, dr7;

  __dr7 = (len | type) & 0xf;
  __dr7 <<= (16 + i * 4);
  __dr7 |= 0x2 << (i * 2);

  dr7 |= __dr7 | DR_LE | DR_GE;

  set_dr(1, addr);
  set_dr(7, dr7);

  debug_print("DR breakpoints set");
  
  return 0;
}
