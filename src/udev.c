#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libexplain/ioctl.h>
#include <limits.h>
#include "lib.h"
#include "udev.h"

static int fd;

static int close_fd() { return close(fd); }

int emit(int fd, int type, int code, int val, int emit_syn) {
  struct input_event ie;

  memset(&ie, 0, sizeof(struct input_event));
  ie.type = type;
  ie.code = code;
  ie.value = val;
  gettimeofday(&ie.time, NULL);

  if (check_fail(write(fd, &ie, sizeof(struct input_event)),
                 "emit: write input_event to uinput fd", &close_fd,
                 "close uinput fd") < 0) {
    return -1;
  }
  if (emit_syn > 0)
    return emit(fd, EV_SYN, SYN_REPORT, 0, 0);
  return 0;
}

int clean_up() {
  if (check_fail(ioctl(fd, UI_DEV_DESTROY), "ioctl: UI_DEV_DESTROY", &close_fd,
                 "close uinput fd") < 0) {
    return -1;
  }
  if (check_fail(close_fd(), "clean up: closing uinput fd", NULL, NULL) < 0) {
    return -1;
  }
  return 0;
}

const int abses[] = {ABS_X, ABS_Y, ABS_Z};
const int btns[] = {BTN_TRIGGER_HAPPY1, BTN_TRIGGER_HAPPY2, BTN_0, BTN_1};

int setup_input(const char *setbit_name, int ev_code, int set_bit, int code) {
  int i;
  if (check_fail(ioctl(fd, UI_SET_EVBIT, ev_code), "ioctl: UI_SET_EVBIT",
                 &close_fd, "close uinput fd") < 0) {
    return -1;
  }
  char desc[50];
  sprintf(desc, "ioctl: %s", setbit_name);
  if (check_fail(ioctl(fd, set_bit, code), desc, &close_fd, "close uinput fd") < 0) {
    return -1;
  }
  return 0;
}

int setup_abses() {
  int i;
  for (i = 0; i < 3; i++) {
    if (setup_input("UI_SET_ABSBIT", EV_ABS, UI_SET_ABSBIT, abses[i]) < 0) {
      return -1;
    }
  }
  return 0;
}

int setup_btns() {
  int i;
  for (i = 0; i < sizeof(btns) / sizeof(int); i++) {
    if (setup_input("UI_SET_KEYBIT", EV_KEY, UI_SET_KEYBIT, btns[i]) < 0) {
      return -1;
    }
  }
  return 0;
}

int udev_setup(void) {
  struct uinput_user_dev udev;
  fd = open("/dev/uinput", O_RDWR);
  if (fd <= 0) {
    fprintf(stderr, "Couldn't open /dev/uinput\n");
    return -1;
  }

  if (setup_abses() < 0) return -1;
  if (setup_btns() < 0) return -1;

  memset(&udev, 0, sizeof(struct uinput_user_dev));
  udev.id.bustype = BUS_GAMEPORT;
  udev.ff_effects_max = 0;
  for (int i = 0; i < ABS_MAX; i++) {
    udev.absmin[i] = SHRT_MIN;
    udev.absmax[i] = SHRT_MAX;
  }
  strcpy(udev.name, "YCLWheel");

  if (check_fail(write(fd, &udev, sizeof(struct uinput_user_dev)), "writing udev", &close_fd, "close uinput fd") < 0) {
    return -1;
  }

  if (check_fail(ioctl(fd, UI_DEV_CREATE), "ioctl: UI_DEV_CREATE", &close_fd, "close uinput fd") < 0) {
    return -1;
  }
  return 0;
}
