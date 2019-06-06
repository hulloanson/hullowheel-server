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
#include "lib.h"
#include "udev.h"
#include "server.h"

// int run_commands() {
//   char command = getchar();
//   switch (command) {
//   case 's':
//     return emulate_press() < 0 ? -1 : 1;
//   case EOF:
//     fprintf(stderr, "Couldn't getchar(). wth.\n");
//     return -1;
//   case 'x':
//     printf("Exiting.\n");
//     return 0;
//   default:
//     return 1;
//   }
// }


// TODO: catch interrupt https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
int main(int argc, char **argv) {
  if (setup() < 0) {
    return -1;
  }

  char buf[1];
  int run = 1;

  pthread_t server;
  if (pthread_create(&server, NULL, &serve, ))
  while (run == 1) {
    run = run_commands();
  }
  return clean_up();
}