#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libexplain/ioctl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <errno.h>
#include "lib.h"
#include "server.h"
#include "wheel.h"

struct server* make_server(int port) {
  struct server *srv = (struct server*) calloc(1, sizeof(struct server));
  srv->port = port;
  srv->should_run = 1;
  return srv;
}

float get_float(char *bytes, int offset) {
  float value = 0.0;
  bcopy(bytes + offset, &value, 4);
  return value;
}

signed int normalize_rotation(float raw, int raw_min, int raw_max, int expected_min, int expected_max) {
  float amplified = (raw - raw_min) / (raw_max - raw_min) * (expected_max - expected_min) + expected_min;
  return (signed int) floor(amplified);
}

struct frame* parse_data(char *bytes) {
  struct frame *frame = (struct frame*)calloc(1, sizeof(struct frame));
  // float wheel = get_float(bytes, WHEEL_OFFSET);
  // float gas = get_float(bytes, GAS_OFFSET);
  // float brake = get_float(bytes, BRAKE_OFFSET);
  // printf("raw: wheel: %f; gas: %f; brake: %f\n", wheel, gas, brake);

  frame->wheel = normalize_rotation(get_float(bytes, WHEEL_OFFSET), WHEEL_MIN_VALUE, WHEEL_MAX_VALUE, WHEEL_EXPECTED_MIN, WHEEL_EXPECTED_MAX);
  
  frame->gas = normalize_rotation(get_float(bytes, GAS_OFFSET), GAS_MIN_VALUE, GAS_MAX_VALUE, GAS_EXPECTED_MIN, GAS_EXPECTED_MAX);

  frame->brake = normalize_rotation(get_float(bytes, BRAKE_OFFSET), BRAKE_MIN_VALUE, BRAKE_MAX_VALUE, BRAKE_EXPECTED_MIN, BRAKE_EXPECTED_MAX);
  bcopy(bytes + BTNS_OFFSET, frame->btns, BTNS_COUNT);
  // printf("normalized: wheel: %d; gas: %d; brake: %d\n", frame->wheel, frame->gas, frame->brake);
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
  free(frame);
  return 0;
}

int close_server(struct server *srv) {
  printf("Closing server...\n");
  srv->should_run = 0;
  int res = 0;
  res = close(srv->fd);
  free(srv);
  printf("Closed server.\n");
  return res;
}

int serve(struct server *srv, struct vwheel *wheel) {
  // 3 axes, 40 buttons i.e. 3 * 4 bytes + 24 bytes = 36 bytes
  int actual_len;
  char buf[EXPECTED_LEN];
  printf("Serving.\n");
  int res;
  while (srv->should_run) {
    res = recvfrom(srv->fd, buf, EXPECTED_LEN, 0, 0, 0);
    if (res != 0 && errno == EAGAIN)
      continue;
    if (check_fail(res, "receive data over UDP") < 0) {
      close_server(srv);
      return -1;
    }
    if (emit_frame(wheel, parse_data(buf)) < 0) {
      close_server(srv);
      return -1;
    }
  }
  printf("Stopped serving.\n");
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
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  if (check_fail(setsockopt(srv->fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)), "set server timeout") < 0) {
    close_server(srv);
    return -1;
  }
  return 0;
}