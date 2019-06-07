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

void* serve_in_thread(void *args) {
  int *ret = (int*)args;
  *ret = serve((struct server*) (args + 1), (struct vwheel*) (args + 2));
  return ret;
}

void sig_handler(int sig) {
  signal(sig, SIG_IGN);
  printf("Interrupt detected. Exiting...");
  close_server(srv);
}

// TODO: catch interrupt https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
int main(int argc, char **argv) {
  wheel = make_vwheel("HulloWheel");
  if (setup_wheel(wheel) < 0) {
    return -1;
  }

  srv = make_server(20000);
  if (setup_server(srv) < 0) {
    return -1;
  }

  pthread_t server_thread;
  void *args;
  memset(args, 0, sizeof(void*) * 3);
  int *ret;
  void *ret_arg = args;
  args = ret;
  void *srv_arg = args + 1;
  srv_arg = srv;
  void *wheel_arg = args + 2;
  wheel_arg = wheel;

  int res = pthread_create(&server_thread, NULL, serve_in_thread, args);
  if (res != 0) {
    fprintf(stderr, "Could not create thread for server. error code was %d.\n", res);
    return -1;
  }

  pthread_join(server_thread, NULL);

  printf("Shutdown procedure completed. Bye.\n");

  return -1;
}