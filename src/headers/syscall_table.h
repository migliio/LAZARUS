#ifndef SYSCALL_H
#define SYSCALL_H

void *sys_call_table_retrieve(void);

int set_sct_rw(unsigned long table_ptr);
int set_sct_ro(unsigned long table_ptr);

#endif
