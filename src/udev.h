#ifndef VWHEEL_UDEV
#define VWHEEL_UDEV

int emit(int fd, int type, int code, int val, int emit_syn);

int clean_up();

int udev_setup();

#endif
