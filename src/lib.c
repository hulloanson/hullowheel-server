#include "lib.h"
#include <stdio.h>

int check_fail(int res, const char *what, int (*cleanup_func)(void),
               const char *cleanup_desc) {
  if (res < 0) {
    fprintf(stderr, "Couldn't perform %s. Error code was %d\n", what, res);
    if (cleanup_func != NULL && cleanup_func() < 0) {
      fprintf(stderr,
              "An error happened, and an error happened while cleaning "
              "up: %s",
              cleanup_desc);
      return -1;
    }
  }
  return res;
}