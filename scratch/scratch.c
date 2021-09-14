#include <gba.h>

#define SCREENBUFFER       ((volatile uint16_t*)0x06000000)

int main() {
  REG_DISPCNT = MODE_3 | BG2_ENABLE;

  for (int pix_idx = 0; pix_idx < SCREEN_WIDTH * SCREEN_HEIGHT; ++pix_idx) {
    SCREENBUFFER[pix_idx] = 0x001F;
  }

  while(1) {}

  return 0;
}
