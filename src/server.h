#ifndef VWHEEL_SERVER
#define VWHEEL_SERVER

#define WHEEL_OFFSET 0
#define GAS_OFFSET 4
#define BRAKE_OFFSET 8
#define BTNS_OFFSET 12
#define BTNS_COUNT 40

#define WHEEL_MAX_VALUE 180
#define WHEEL_MIN_VALUE 0

#define GAS_MIN_VALUE 0
#define GAS_MAX_VALUE 200

#define BRAKE_MIN_VALUE 0
#define BRAKE_MAX_VALUE 200

#include "wheel.h"

struct server {
  int port;
  int fd;
  int should_run;
};

struct frame {
  signed int wheel;
  signed int gas;
  signed int brake;
  char btns[40];
};

int serve(struct server *srv, struct vwheel *wheel);

#endif