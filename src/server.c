#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <zlib.h>
#include <errno.h>
#include <stdbool.h>
#include "lib.h"
#include "server.h"
#include "wheel.h"
#include "macrologger.h"

#define WHEEL_OFFSET 0
#define WHEEL_LEN 2

#define GAS_OFFSET WHEEL_OFFSET + WHEEL_LEN
#define GAS_LEN 1

#define BRAKE_OFFSET GAS_OFFSET + GAS_LEN
#define BRAKE_LEN 1

#define BTNS_OFFSET BRAKE_OFFSET + BRAKE_LEN
#define BTNS_COUNT 24
#define BUTTON_LEN BTNS_COUNT / 8

#define EXPECTED_LEN WHEEL_LEN + GAS_LEN + BRAKE_LEN + BUTTON_LEN

typedef struct {
  int16_t wheel;
  uint8_t gas;
  uint8_t brake;
  uint8_t btns[BTNS_COUNT];
} state;

struct server* make_server(int port) {
  struct server *srv = (struct server*) calloc(1, sizeof(struct server));
  srv->port = port;
  return srv;
}

int get_btn_state(char *btn_state, int button) { // `button`: 0 - BTNS_COUNT - 1
  int byte_offset = button / 8;
  int bit_offset = button % 8;
  int value = btn_state[byte_offset] >> bit_offset & 1;
  LOG_DEBUG("reading %d-th button's state. byte_offset: %d; bit_offset: %d; value: %d", button, byte_offset, bit_offset, value);
  return value;
}

void print_out_of_range(const char *what, int min, int max, int actual) {
  LOG_DEBUG("Failed to parse data. %s value out of range. Expected %d - %d, got %d", what, min, max, actual);
}

int parse_data(char *bytes, state *s) {
  // wheel: expects -150 to 150
  s->wheel = *((uint16_t *) (bytes + WHEEL_OFFSET));
  if (s->wheel < WHEEL_MIN_VALUE || s->wheel > WHEEL_MAX_VALUE) {
    print_out_of_range("Wheel", WHEEL_MIN_VALUE, WHEEL_MAX_VALUE, s->wheel);
    return -1;
  }
  s->gas = *((uint8_t *)(bytes + GAS_OFFSET));
  if (s->gas < GAS_MIN_VALUE || s->gas > GAS_MAX_VALUE) {
    print_out_of_range("Gas", GAS_MIN_VALUE, GAS_MAX_VALUE, s->gas);
    return -1;
  }
  s->brake = *((uint8_t *)(bytes + BRAKE_OFFSET));
  if (s->brake < BRAKE_MIN_VALUE || s->brake > BRAKE_MAX_VALUE) {
    print_out_of_range("Brake", BRAKE_MIN_VALUE, BRAKE_MAX_VALUE, s->brake);
    return -1;
  }
  LOG_DEBUG("first button byte: %u", (unsigned char)bytes[BTNS_OFFSET]);
  LOG_DEBUG("second button byte: %u", (unsigned char)bytes[BTNS_OFFSET + 1]);
  LOG_DEBUG("third button byte: %u", (unsigned char) bytes[BTNS_OFFSET + 2]);
  for (int i = 0; i < BTNS_COUNT; i++) {
    s->btns[i] = get_btn_state(bytes + BTNS_OFFSET, i) ? 1 : 0;
  }
  return 0;
}

int emit_wheel(vwheel *wheel, int16_t val) {
  if (emit(wheel, EV_ABS, ABS_WHEEL, val, 1) < 0) {
    LOG_DEBUG("Failed to emit ABS_WHEEL value %d.", val);
    return -1;
  }
  return 0;
}

void print_execute_state_failed() {
  LOG_ERROR("Failed to execute virtual wheel commands");
}

int execute_state(vwheel *wheel, state s) {
  if (emit_wheel(wheel, s.wheel) < 0) {
    print_execute_state_failed();
    return -1;
  }
  if (emit_gas(wheel, s.gas) < 0){ 
    print_execute_state_failed();
    return -1;
  }
  if (emit_brake(wheel, s.brake) < 0) {
    print_execute_state_failed();
    return -1;
  }
  int i;
  for (i = 0; i < BTNS_COUNT; i++) {
    if (emit_btn(wheel, i, s.btns[i]) < 0) {
      print_execute_state_failed();
      return -1;
    }
  }
  return 0;
}

int close_server(struct server *srv) {
  LOG_DEBUG("Closing server...");
  int res = 0;
  res = close(srv->fd);
  LOG_DEBUG("Closed server. close() said %d. errno was %d", res, res == 0 ? 0 : errno);
  return res;
}

bool prevConnected = false;
bool connected = false;

int serve(struct server *srv, vwheel *wheel, int *should_run) {
  char in[EXPECTED_LEN];
  int res;
  while (*should_run == 1) {
    // Get size
    res = recvfrom(srv->fd, in, EXPECTED_LEN, 0, 0, 0);
    prevConnected = connected;
    if (res != 0 && errno == EAGAIN) {
      connected = false;
      if (prevConnected != false) {
        LOG_INFO("Disconnected.");
      }
      return -2;
    }
    // received normally, without timeout
    connected = true;
    if (prevConnected != true) {
      LOG_INFO("Connected");
    }
    int size = res;
    if (check_fail(res, "receive data over UDP") < 0) {
      return -1;
    }
    state s;
    parse_data(in, &s);
    LOG_DEBUG("wheel: %d; gas: %d; brake: %d",
      s.wheel, s.gas, s.brake
    );
    execute_state(wheel, s);
  }
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