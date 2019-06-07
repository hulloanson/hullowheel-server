#ifndef VWHEEL_UDEV
#define VWHEEL_UDEV

struct vwheel {
  int fd;
  char name[50];
};

int emit(struct vwheel *wheel, int type, int code, int val, int emit_syn);

int remove_wheel(struct vwheel *wheel);

int setup_wheel(struct vwheel *wheel);

#endif
