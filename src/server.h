#ifndef VWHEEL_SERVER
#define VWHEEL_SERVER

#include <limits.h>

#include <string.h>
#include <stdint.h>
#include "wheel.h"

struct server {
  int port;
  int fd;
};

struct server* make_server(int port);

typedef struct {
  int16_t wheel;
  uint8_t gas;
  uint8_t brake;
  uint8_t btns_gp1;
  uint8_t btns_gp2;
  uint8_t btns_gp3;
} frame;

int serve(struct server *srv, vwheel *wheel, int *should_run);

int setup_server(struct server *srv);

int close_server(struct server *srv);

#endif