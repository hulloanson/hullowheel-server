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

struct vwheel *wheel;
struct server *srv;

struct server_in_out {
  int *ret;
  struct server *srv;
  struct vwheel *wheel;
};

void* serve_in_thread(void *arg) {
  struct server_in_out *srv_in_out = (struct server_in_out*) arg;
  int *ret = srv_in_out->ret;
  *ret = serve(srv_in_out->srv, srv_in_out->wheel);
  return ret;
}

void int_handler(int sig, siginfo_t *siginfo, void *context) {
  signal(sig, SIG_IGN);
  printf("Interrupt detected. Exiting...\n");
  close_server(srv);
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
  if( register_sigint() < 0) {
    fprintf(stderr, "Couldn't register sigint handler.\n");
    return -1;
  }
  wheel = make_vwheel("HulloWheel");
  if (setup_wheel(wheel) < 0) {
    return -1;
  }

  srv = make_server(SERVER_PORT);
  if (setup_server(srv) < 0) {
    return -1;
  }

  pthread_t server_thread;
  struct server_in_out srv_in_out;
  srv_in_out.srv = srv;
  srv_in_out.wheel = wheel;
  int ret = 0;
  srv_in_out.ret = &ret;

  int res = pthread_create(&server_thread, NULL, serve_in_thread, &srv_in_out);
  if (res != 0) {
    fprintf(stderr, "Could not create thread for server. error code was %d.\n", res);
    remove_wheel(wheel);
    return -1;
  }

  pthread_join(server_thread, NULL);

  printf("Server thread closed.\n");
  printf("Now removing wheel...\n");
  remove_wheel(wheel);

  printf("Shutdown procedure completed. Bye.\n");

  return -1;
}