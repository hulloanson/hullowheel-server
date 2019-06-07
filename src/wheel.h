#ifndef VWHEEL_UDEV
#define VWHEEL_UDEV

#include <string.h>

struct vwheel {
  int fd;
  char name[50];
};

struct vwheel* make_vwheel(const char *name) {
  struct vwheel *wheel;
  memset(wheel, 0, sizeof(struct vwheel));
  strcpy(wheel->name, name);
  return wheel;
}

int emit(struct vwheel *wheel, int type, int code, int val, int emit_syn);

int remove_wheel(struct vwheel *wheel);

int setup_wheel(struct vwheel *wheel);

#endif
