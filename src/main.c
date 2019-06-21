#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libexplain/ioctl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include "lib.h"
#include "wheel.h"
#include "server.h"
#include "cmdline.h"
#include "macrologger.h"

struct vwheel *wheel;
struct server *srv;
int *should_run;

struct server_in {
  struct server *srv;
  struct vwheel *wheel;
};

void* serve_in_thread(void *arg) {
  struct server_in *srv_in = (struct server_in*) arg;
  int *ret = (int *) calloc(1, sizeof(int));
  *ret = serve(srv_in->srv, srv_in->wheel, should_run);
  pthread_exit(ret);
}

void int_handler(int sig, siginfo_t *siginfo, void *context) {
  signal(sig, SIG_IGN);
  LOG_INFO("Interrupt detected. Exiting...");
  *should_run = 0;
}

int register_sigint() {
  struct sigaction act;
 
	memset (&act, '\0', sizeof(act));
 
	/* Use the sa_sigaction field because the handles has two additional parameters */
	act.sa_sigaction = &int_handler;
 
	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
	act.sa_flags = SA_SIGINFO;

  return sigaction(SIGINT, &act, NULL);
}

// TODO: catch interrupt https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
int main(int argc, char **argv) {
  struct gengetopt_args_info args_info;
  if (cmdline_parser(argc, argv, &args_info) != 0) {
    exit(1);
  }
  int port = args_info.port_arg;
  should_run = (int *) calloc(1, sizeof(int));
  *should_run = 1;
  if( register_sigint() < 0) {
    LOG_ERROR("Couldn't register sigint handler.");
    return -1;
  }
  wheel = make_vwheel("HulloWheel");
  if (setup_wheel(wheel) < 0) {
    return -1;
  }

  pthread_t *server_thread;
  struct server_in srv_in;
  srv_in.wheel = wheel;

  int *srv_exit = (int *) calloc(1, sizeof(int));
  *srv_exit = -2;
  int first = 1;
  while (*srv_exit == -2) {
    LOG_INFO("%s server thread", first-- == 1 ? "Creating": "Re-creating");
    server_thread = (pthread_t *) calloc(1, sizeof(pthread_t));
    srv = make_server(port);
    if (setup_server(srv) < 0) {
      return -1;
    }
    srv_in.srv = srv;
    int res = pthread_create(server_thread, NULL, serve_in_thread, &srv_in);
    if (res != 0) {
      LOG_ERROR("Could not create thread for server. error code was %d.\n", res);
      remove_wheel(wheel);
      return -1;
    }
    pthread_join(*server_thread, (void **)&srv_exit);

    res = check_fail(close_server(srv), "closing server");
    free(srv);
    free(server_thread);
    if (res == -1) break;
  }

  LOG_INFO("Server thread closed.");
  LOG_INFO("Now removing wheel...");
  remove_wheel(wheel);

  LOG_INFO("Shutdown procedure completed. Bye.");

  return -1;
}