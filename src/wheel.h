#ifndef VWHEEL_WHEEL
#define VWHEEL_WHEEL

#include <string.h>
#include <stdint.h>

#define WHEEL_MAX_VALUE 150
#define WHEEL_MIN_VALUE -150

#define GAS_MIN_VALUE 0
#define GAS_MAX_VALUE 120

#define BRAKE_MIN_VALUE 0
#define BRAKE_MAX_VALUE 120

typedef struct {
  int fd;
  char name[50];
} vwheel;

vwheel* make_vwheel(const char *name);

int emit(vwheel *wheel, int type, int code, int val, int emit_syn);

int emit_btn(vwheel *wheel, int btn, uint8_t val);

int emit_brake(vwheel *wheel, uint8_t val);

int emit_gas(vwheel *wheel, uint8_t val);

int remove_wheel(vwheel *wheel);

int setup_wheel(vwheel *wheel);

#endif
