#include "lib.h"
#include <stdio.h>

int check_fail(int res, const char *what) {
  if (res < 0) {
    fprintf(stderr, "Couldn't perform %s. Error code was %d\n", what, res);
  }
  return res;
}