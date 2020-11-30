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
#include <strings.h>
#include <unistd.h>
#include <stdint.h>
#include "macrologger.h"

vwheel* make_vwheel(const char *name) {
  vwheel *wheel = (vwheel*)calloc(1, sizeof(vwheel));
  strcpy(wheel->name, name);
  return wheel;
}

int close_wheel(vwheel *wheel) {
  LOG_DEBUG("Closing wheel %s", wheel->name);
  return check_fail(close(wheel->fd), "close wheel by fd");
}

int add_input(vwheel *wheel, const char *setbit_name, int ev_code,
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

int add_wheel_btns(vwheel *wheel) {
  LOG_DEBUG("Adding buttons to the wheel");
  int i;
  for (i = BTN_TRIGGER_HAPPY; i <= BTN_TRIGGER_HAPPY40; i++) {
    if (check_fail(add_input(wheel, "UI_SET_KEYBIT", EV_KEY, UI_SET_KEYBIT, i),
                   "add wheel button") < 0) {
      close_wheel(wheel);
      return -1;
    }
  }
  LOG_DEBUG("Done adding buttons to the wheel");
  return 0;
}

int add_wheel_abs(vwheel *wheel, int code) {
  if (check_fail(add_input(wheel, "UI_SET_KEYBIT", EV_ABS, UI_SET_ABSBIT, code),
                 "add wheel abs") < 0) {
    close_wheel(wheel);
    return -1;
  }
  return 0;
}

int add_wheel_w_pedals(vwheel *wheel) {
  LOG_DEBUG("Actually making the wheel and adding pedals");
  if (add_wheel_abs(wheel, ABS_WHEEL) < 0)
    return -1;
  if (add_wheel_abs(wheel, ABS_GAS) < 0)
    return -1;
  if (add_wheel_abs(wheel, ABS_BRAKE) < 0)
    return -1;
  LOG_DEBUG("Done making the wheel and its pedals.");
  return 0;
}

int emit(vwheel *wheel, int type, int code, int val, int emit_syn) {
  if (code >= BTN_TRIGGER_HAPPY1 && code <= BTN_TRIGGER_HAPPY40) {
    LOG_DEBUG("emit: Emitting button event. value: %d", val);
  } 
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

int emit_gas(vwheel *wheel, uint8_t val) {
  if (emit(wheel, EV_ABS, ABS_GAS, val, 1) < 0){ 
    LOG_DEBUG("Failed to emit ABS_GAS value %d.", val);
    return -1;
  }
  return 0;
}

int emit_brake(vwheel *wheel, uint8_t val) {
  if (emit(wheel, EV_ABS, ABS_BRAKE, val, 1) < 0) {
    LOG_DEBUG("Failed to emit ABS_BRAKE value %d.", val);
    return -1;
  }
  return 0;
}

int emit_btn(vwheel *wheel, int btn, uint8_t val) {
  if (emit(wheel, EV_KEY, BTN_TRIGGER_HAPPY1 + btn, val, 1) < 0) {
    LOG_DEBUG("Failed to emit value %d for btn %d.", val, btn);
    return -1;
  }
}

int get_wheel_permit(vwheel *wheel) {
  LOG_DEBUG("Getting wheel fd from uinput");
  // Obtain the fd for the wheel
  wheel->fd = open("/dev/uinput", O_RDWR);
  if (wheel->fd <= 0) {
    LOG_ERROR("Couldn't open /dev/uinput");
    return -1;
  }
  LOG_DEBUG("Done getting wheel fd from uinput");
  return 0;
}

int construct_wheel(vwheel *wheel) {
  if (add_wheel_w_pedals(wheel) < 0) return -1;
  if (add_wheel_btns(wheel) <0) return -1;
  return 0;
}

int register_wheel(vwheel *wheel) {
  LOG_DEBUG("Registering wheel");
  struct uinput_user_dev udev;
  memset(&udev, 0, sizeof(struct uinput_user_dev));

  // Fill info
  udev.id.bustype = BUS_GAMEPORT;
  udev.ff_effects_max = 0;
  udev.absmin[ABS_WHEEL] = WHEEL_MIN_VALUE;
  udev.absmax[ABS_WHEEL] = WHEEL_MAX_VALUE;
  udev.absmin[ABS_GAS] = GAS_MIN_VALUE;
  udev.absmax[ABS_GAS] = GAS_MAX_VALUE;
  udev.absmin[ABS_BRAKE] = BRAKE_MIN_VALUE;
  udev.absmax[ABS_BRAKE] = BRAKE_MAX_VALUE;
  strcpy(udev.name, wheel->name);

  // Register
  if (check_fail(write(wheel->fd, &udev, sizeof(struct uinput_user_dev)),
                 "registering wheel to its fd") < 0) {
    return -1;
  }
  LOG_DEBUG("Done registering wheel");
  return 0;
}

int confirm_wheel(vwheel *wheel) {
  LOG_DEBUG("Actually ask uinput to create the wheel");
  // Actually tell uinput to create the device
  if (check_fail(ioctl(wheel->fd, UI_DEV_CREATE), "ioctl: UI_DEV_CREATE") < 0) {
    return -1;
  }
  LOG_DEBUG("uinput said it has now created the wheel");
  return 0;
}

int setup_wheel(vwheel *wheel) {
  LOG_DEBUG("Setting up the virtual wheel");
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
  LOG_DEBUG("Done setting up the virtual wheel");
  return 0;
}

int remove_wheel(vwheel *wheel) {
  LOG_DEBUG("Removing the virtual wheel...");
  int res = 0;
  res = check_fail(ioctl(wheel->fd, UI_DEV_DESTROY), "ioctl: UI_DEV_DESTROY");
  res |= close_wheel(wheel);
  free(wheel);
  LOG_DEBUG("Removed.");
  return res;
}