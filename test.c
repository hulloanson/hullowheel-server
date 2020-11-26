#include <stdint.h>
#include <stdio.h>

typedef struct {
  int16_t wheel;
  uint8_t gas;
  uint8_t brake;
  uint8_t btns_gp1;
  uint8_t btns_gp2;
  uint8_t btns_gp3;
} frame;


typedef struct {
  int16_t wheel;
  uint8_t gas;
  uint8_t brake;
  uint8_t btns[24];
} state;

int main() {
	printf("sizeof(frame): %ld\n", sizeof(frame));
	printf("sizeof(state): %ld\n", sizeof(state));
	return 0;
}

