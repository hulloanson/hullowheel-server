#include <linux/uinput.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>
#include "mock.h"

int emulate_press(int fd) {
  int delay = 5; // s
  int duration = 1000; //ms
  int fps = 50;
  int i = fps * duration / 1000;
  printf("Pressing in 5 secs\n");
  sleep(delay);
  printf("Press\n");
  while (i-- > 0) {
    if (emit(fd, EV_ABS, ABS_X, 10000, 1) < 0) {
      return -1;
    }
    usleep(duration / fps * 1000);
  }
  printf("Release\n");
  return emit(fd, EV_ABS, ABS_X, 0, 1);
}