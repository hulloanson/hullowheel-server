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
#include "lib.h"

int fd, pipe_r, pipe_w;

int close_fd() {
  return close(fd);
}

int close_pipes() {
  if (close(pipe_w) < 0) {
    return -1;
  }
  return close(pipe_r);
}

int serve(int pipe_r, int pipe_w) {
  const int expected = 40;
  int actual;
  char buf[40];
  char pipe_r_buf[10];
  bzero(pipe_r_buf, 10);
  while (1) {
    bzero(buf, 40);
    if (check_fail((actual = recvfrom(fd, buf, expected, 0, 0, 0)),
                   "receiving data", close_fd, "close socket fd") < 0) {
      return -1;
    }
    // Send buf over pipe_w
    if (check_fail(write(pipe_w, buf, 40), "send received data over pipe", close_pipes, "close pipes") < 0) {
      return -1;
    }
    if (check_fail(read(pipe_r, pipe_r_buf, 10), "read from parent", close_pipes, "close pipes")< 0) {
      return -1;
    }
    if (strcmp("end", pipe_r_buf) == 0) {
      break;
    }
  }
  return 0;
}

int setup() {
  const int port = 20000;

  if (check_fail(fd = socket(AF_INET, SOCK_DGRAM, 0), "create udp socket", 0, 0) < 0) {
    return -1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (check_fail(bind(fd, (struct sockaddr *)&addr, sizeof(addr)), "bind to 0.0.0.0:20000", close_fd, "close socket fd") < 0) {
    return -1;
  }
  return 0;
}
