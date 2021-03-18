#ifndef S_ESC_H
#define S_ESC_H

#include "utils.h"

/* set root escalation flag */
extern int sys_esc_flag;

void do_priv_esc(void);
void undo_priv_esc(void);

#endif
