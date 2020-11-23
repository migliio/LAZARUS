#ifndef SERV_H
#define SERV_H

#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/delay.h>
#include <linux/timer.h>

/* struct for udp kernel thread */
struct kthread_t {
  struct task_struct *curr;
  struct socket *sock;
  struct sockaddr_in addr;
  int running;
};

int server_start(void);
void server_stop(void);
int server_rcv(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len);
int server_do(const char *command, struct sockaddr_in *addr);

#endif
