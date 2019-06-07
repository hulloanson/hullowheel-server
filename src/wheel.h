#ifndef VWHEEL_UDEV
#define VWHEEL_UDEV

struct vwheel {
  int fd;
  char name[50];
};

int emit(struct vwheel *wheel, int type, int code, int val, int emit_syn);

int clean_up();

int udev_setup();

#endif
