#include "dr_breakpoints.h"
#include "utils.h"

dr_bp bp;

int reg_dr_bp(unsigned long addr, int type, int len)
{
  int i;
  unsigned long dr7 = 0;

  static int init = 0;

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

  dr7 = (len | type) & 0xf;
  dr7 <<= (16 + i * 4);
  dr7 |= 0x2 << (i * 2);

  /* bp.dr7 |= DR_GD; */
  bp.dr7 |= dr7 | DR_LE | DR_GE;

  // on_each_cpu_set_dr(i, bp.dr[i]);
  
  return 0;
}

int unreg_dr_bp(unsigned long addr)
{
  int i;
  
  return 0;
}
