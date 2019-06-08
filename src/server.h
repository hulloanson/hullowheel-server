#ifndef VWHEEL_SERVER
#define VWHEEL_SERVER

#include <limits.h>

#define WHEEL_OFFSET 0
#define GAS_OFFSET 4
#define BRAKE_OFFSET 8
#define BTNS_OFFSET 12
#define BTNS_COUNT 24

#define EXPECTED_LEN 3 * 4 + BTNS_COUNT

#define WHEEL_MAX_VALUE 220
#define WHEEL_MIN_VALUE -40
#define WHEEL_EXPECTED_MAX SHRT_MAX
#define WHEEL_EXPECTED_MIN SHRT_MIN

#define GAS_MIN_VALUE 0
#define GAS_MAX_VALUE 1000
#define GAS_EXPECTED_MAX SHRT_MAX
#define GAS_EXPECTED_MIN SHRT_MIN

#define BRAKE_MIN_VALUE 0
#define BRAKE_MAX_VALUE 1000
#define BRAKE_EXPECTED_MAX SHRT_MAX
#define BRAKE_EXPECTED_MIN SHRT_MIN

#include <string.h>
#include "wheel.h"

struct server {
  int port;
  int fd;
  int should_run;
};

struct server* make_server(int port);

struct frame {
  signed int wheel;
  signed int gas;
  signed int brake;
  char btns[BTNS_COUNT];
};

int serve(struct server *srv, struct vwheel *wheel);

int setup_server(struct server *srv);

int close_server(struct server *srv);

#endif