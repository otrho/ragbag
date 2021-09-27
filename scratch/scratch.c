#include <gba.h>

#include "white-all.h"

void init_gfx() {
  // Set the video mode to 0, enable bg 0, enable objects.
  SetMode(MODE_0 | BG0_ENABLE | OBJ_ENABLE);

  // Initialised bg0 parameters:
  // - char block 0
  // - screen block 28
  // - 64x64 tiles, 512x512 px
#define SCR_BASE_BLOCK 28
  BGCTRL[0] = CHAR_BASE(0) | SCREEN_BASE(SCR_BASE_BLOCK) | BG_SIZE_3;

  // Init bg0 scroll to the origin.
  REG_BG0HOFS = 0;
  REG_BG0VOFS = 0;

  CpuFastSet(white_allTiles, CHAR_BASE_ADR(0), 512 | COPY32);

  CpuFastSet(white_allPal, BG_PALETTE, 16 | COPY16);

  u16* screen_block = SCREEN_BASE_BLOCK(SCR_BASE_BLOCK);
  for (int block_idx = 0; block_idx < 4; block_idx++) {
    for (int tile_idx = 0; tile_idx < 1024; tile_idx++) {
      screen_block[block_idx * 1024 + tile_idx] = (u16)(tile_idx & 0x3f);
    }
  }
}

int main() {
  irqInit();
  irqEnable(IRQ_VBLANK);
  REG_IME = 1;

  init_gfx();

  u16 scroll_x = 0;
  u16 scroll_y = 0;

  while(1) {
    VBlankIntrWait();

    scanKeys();
    u16 keys_up = keysUp();

    if (keys_up & KEY_START) {
      break;
    }

    if (keys_up & KEY_UP) {
      scroll_y -= 32;
    }
    if (keys_up & KEY_DOWN) {
      scroll_y += 32;
    }
    if (keys_up & KEY_LEFT) {
      scroll_x -= 32;
    }
    if (keys_up & KEY_RIGHT) {
      scroll_x += 32;
    }

    REG_BG0HOFS = scroll_x;
    REG_BG0VOFS = scroll_y;
  }

  Stop();
  return 0;
}
