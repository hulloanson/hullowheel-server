#include "wheel.h"
#include "lib.h"
#include <fcntl.h>
#include <libexplain/ioctl.h>
#include <limits.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct vwheel* make_vwheel(const char *name) {
  struct vwheel *wheel;
  memset(wheel, 0, sizeof(struct vwheel));
  strcpy(wheel->name, name);
  return wheel;
}

int close_wheel(struct vwheel *wheel) {
  printf("Closing wheel %s", wheel->name);
  return check_fail(close(wheel->fd), "close wheel by fd");
}

int add_input(struct vwheel *wheel, const char *setbit_name, int ev_code,
              int set_bit, int code) {
  int i, res;
  if (check_fail(ioctl(wheel->fd, UI_SET_EVBIT, ev_code),
                 "ioctl: UI_SET_EVBIT") < 0) {
    close_wheel(wheel);
    return -1;
  }
  char desc[50];
  sprintf(desc, "ioctl: %s", setbit_name);
  if (check_fail(ioctl(wheel->fd, set_bit, code), desc) < 0) {
    close_wheel(wheel);
    return -1;
  }
  return 0;
}

int add_wheel_btns(struct vwheel *wheel) {
  printf("Adding buttons to the wheel\n");
  int i;
  for (i = BTN_TRIGGER_HAPPY; i <= BTN_TRIGGER_HAPPY40; i++) {
    if (check_fail(add_input(wheel, "UI_SET_KEYBIT", EV_KEY, UI_SET_KEYBIT, i),
                   "add wheel button") < 0) {
      close_wheel(wheel);
      return -1;
    }
  }
  printf("Done adding buttons to the wheel\n");
  return 0;
}

int add_wheel_abs(struct vwheel *wheel, int code) {
  if (check_fail(add_input(wheel, "UI_SET_KEYBIT", EV_ABS, UI_SET_ABSBIT, code),
                 "add wheel abs") < 0) {
    close_wheel(wheel);
    return -1;
  }
  return 0;
}

int add_wheel_w_pedals(struct vwheel *wheel) {
  printf("Actually making the wheel and adding pedals\n");
  if (add_wheel_abs(wheel, ABS_WHEEL) < 0)
    return -1;
  if (add_wheel_abs(wheel, ABS_GAS) < 0)
    return -1;
  if (add_wheel_abs(wheel, ABS_BRAKE) < 0)
    return -1;
  printf("Done making the wheel and its pedals.\n");
  return 0;
}

int emit(struct vwheel *wheel, int type, int code, int val, int emit_syn) {
  struct input_event ie;

  memset(&ie, 0, sizeof(struct input_event));
  ie.type = type;
  ie.code = code;
  ie.value = val;
  gettimeofday(&ie.time, NULL);

  if (check_fail(write(wheel->fd, &ie, sizeof(struct input_event)),
                 "emit: write input_event to uinput fd") < 0) {
    close_wheel(wheel);
    return -1;
  }
  if (emit_syn > 0)
    return emit(wheel, EV_SYN, SYN_REPORT, 0, 0);
  return 0;
}

int get_wheel_permit(struct vwheel *wheel) {
  // Obtain the fd for the wheel
  wheel->fd = open("/dev/uinput", O_RDWR);
  if (wheel->fd <= 0) {
    fprintf(stderr, "Couldn't open /dev/uinput\n");
    return -1;
  }
  return 0;
}

int construct_wheel(struct vwheel *wheel) {
  if (add_wheel_w_pedals(wheel) < 0) return -1;
  if (add_wheel_btns(wheel) <0) return -1;
  return 0;
}

int register_wheel(struct vwheel *wheel) {
  struct uinput_user_dev udev;
  memset(&udev, 0, sizeof(struct uinput_user_dev));

  // Fill info
  udev.id.bustype = BUS_GAMEPORT;
  udev.ff_effects_max = 0;
  for (int i = 0; i < ABS_MAX; i++) {
    udev.absmin[i] = SHRT_MIN;
    udev.absmax[i] = SHRT_MAX;
  }
  strcpy(udev.name, wheel->name);

  // Register
  if (check_fail(write(wheel->fd, &udev, sizeof(struct uinput_user_dev)),
                 "registering wheel to its fd") < 0) {
    return -1;
  }
  return 0;
}

int confirm_wheel(struct vwheel *wheel) {
  // Actually tell uinput to create the device
  if (check_fail(ioctl(wheel->fd, UI_DEV_CREATE), "ioctl: UI_DEV_CREATE") < 0) {
    return -1;
  }
  return 0;
}

int setup_wheel(struct vwheel *wheel) {
  if (get_wheel_permit(wheel) < 0) {
    return -1;
  }

  if (construct_wheel(wheel) < 0) {
    return -1;
  }

  if (register_wheel(wheel) < 0) {
    return -1;
  }

  if (confirm_wheel(wheel) < 0) {
    return -1;
  }

  return 0;
}

int remove_wheel(struct vwheel *wheel) {
  if (check_fail(ioctl(wheel->fd, UI_DEV_DESTROY), "ioctl: UI_DEV_DESTROY") <
      0) {
    return -1;
  }
  return close_wheel(wheel);
}