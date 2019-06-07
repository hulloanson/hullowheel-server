#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libexplain/ioctl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include "lib.h"
#include "server.h"
#include "wheel.h"

struct server* make_server(int port) {
  struct server *srv;
  memset(srv, 0, sizeof(struct server));
  srv->port = port;
  srv->should_run = 1;
  return srv;
}

float get_float(char *bytes, int offset) {
  float *value;
  memset(value, 0, sizeof(float));
  bcopy(bytes + offset, value, 4);
  return *value;
}

signed int normalize_rotation(float raw, int raw_min, int raw_max) {
  float amplified = raw * (raw_max - raw_min) + raw_min;
  return (signed int) floor(amplified);
}

struct frame* parse_data(char *bytes) {
  struct frame *frame;
  memset(frame, 0, sizeof(struct frame));
  frame->wheel = normalize_rotation(get_float(bytes, WHEEL_OFFSET), WHEEL_MIN_VALUE, WHEEL_MAX_VALUE);
  frame->gas = normalize_rotation(get_float(bytes, GAS_OFFSET), GAS_MIN_VALUE, GAS_MAX_VALUE);
  frame->brake = normalize_rotation(get_float(bytes, BRAKE_OFFSET), BRAKE_MIN_VALUE, BRAKE_MAX_VALUE);
  bcopy(bytes + BTNS_OFFSET, frame->btns, 40);
  return frame;
}

int emit_frame(struct vwheel *wheel, struct frame *frame) {
  if (emit(wheel, EV_ABS, ABS_WHEEL, frame->wheel, 1) < 0) {
    return -1;
  }
  if (emit(wheel, EV_ABS, ABS_GAS, frame->gas, 1) < 0){ 
    return -1;
  }
  if (emit(wheel, EV_ABS, ABS_BRAKE, frame->brake, 1) < 0) {
    return -1;
  }
  int i;
  for (i = 0; i < BTNS_COUNT; i++) {
    if (emit(wheel, EV_KEY, BTN_TRIGGER_HAPPY1 + i, frame->btns[i], 1) < 0) {
      return -1;
    }
  }
  return 0;
}

int close_server(struct server *srv) {
  srv->should_run = 0;
  return close(srv->fd);
}

int serve(struct server *srv, struct vwheel *wheel) {
  // 3 axes, 40 buttons i.e. 3 * 4 bytes + 40 bytes = 52 bytes
  const int expected_len = 52;
  int actual_len;
  char buf[expected_len];
  while (srv->should_run) {
    if (check_fail(recvfrom(srv->fd, buf, expected_len, 0, 0, 0), "receive data over UDP") < 0) {
      close_server(srv);
      return -1;
    }
    if (emit_frame(wheel, parse_data(buf)) < 0) {
      close_server(srv);
      return -1;
    }
  }
  close_server(srv);
  return 0;
}

int setup_server(struct server *srv) {
  if (check_fail(srv->fd = socket(AF_INET, SOCK_DGRAM, 0), "create udp socket") < 0) {
    return -1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(srv->port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (check_fail(bind(srv->fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)), "bind to 0.0.0.0:20000") < 0) {
    return -1;
  }
  return 0;
}