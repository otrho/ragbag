#include <gba.h>

#include "white-all.h"

void init_gfx() {
  // Set the video mode to 0, enable bg 1, enable objects.
  SetMode(MODE_0 | BG1_ENABLE | OBJ_ENABLE);

  // Initialised bg1 parameters:
  // - char block 0
  // - screen block 28
  // - 64x64 tiles, 512x512 px
#define SCR_BASE_BLOCK 28
  BGCTRL[1] = CHAR_BASE(0) | SCREEN_BASE(SCR_BASE_BLOCK) | BG_SIZE_3;

  // Init bg1 scroll to the origin.
  REG_BG1HOFS = 0;
  REG_BG1VOFS = 0;

  CpuFastSet(white_allTiles, CHAR_BASE_ADR(0) + 32, 512 | COPY32);

  CpuFastSet(white_allPal, BG_PALETTE, 16 | COPY16);

  u16* screen_block = SCREEN_BASE_BLOCK(SCR_BASE_BLOCK);
  for (int tile_idx = 0; tile_idx < 64; tile_idx++) {
    screen_block[tile_idx] = (u16)(tile_idx + 1);
  }

  // Set some pixels in the very first sprite.
  u32 sprite_idx = 1;
  const u32 sprites[2][8] = {
    { 0x11111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x01111111,
      0x00000001 },

    { 0x00000000,
      0x00100100,
      0x01100110,
      0x00011000,
      0x00011000,
      0x01100110,
      0x00100100,
      0x00000000 },
  };

  u32* sprite_ptr = ((u32*)OBJ_BASE_ADR) + (sprite_idx * 8);
  for (u32 row_idx = 0; row_idx < 8; row_idx++) {
    sprite_ptr[row_idx] = sprites[1][row_idx];
  }

  // Set a couple of colours, black and red.
  OBJ_COLORS[0] = RGB5(0, 0, 0);
  OBJ_COLORS[1] = RGB5(31, 0, 0);

  // Set the attributes for the first sprite.
  u16 x = 12, y = 12;
  OBJATTR* sprite_attr = (OBJATTR*)OAM;
  sprite_attr->attr0 = OBJ_Y(y) | ATTR0_NORMAL | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE;
  sprite_attr->attr1 = OBJ_X(x) | ATTR1_SIZE_8;
  sprite_attr->attr2 = OBJ_CHAR(sprite_idx) | ATTR2_PRIORITY(0) | ATTR2_PALETTE(0);
}

int main() {
  irqInit();
  irqEnable(IRQ_VBLANK);
  REG_IME = 1;

  VBlankIntrWait();
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

    REG_BG1HOFS = scroll_x;
    REG_BG1VOFS = scroll_y;
  }

  Stop();
  return 0;
}
