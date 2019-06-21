#include "lib.h"
#include <stdio.h>
#include "macrologger.h"

int check_fail(int res, const char *what) {
  if (res < 0) {
    LOG_ERROR("Couldn't perform %s. Error code was %d", what, res);
  }
  return res;
}