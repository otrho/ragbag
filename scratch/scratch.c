#include <gba.h>


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

  // (1) Create 2 4bpp tiles: 1x basic tile and 1x cross
  const u32 tiles[2][8] = {
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
  CpuFastSet(tiles[0], CHAR_BASE_ADR(0), 16 | COPY16);
  CpuFastSet(tiles[1], CHAR_BASE_ADR(1), 16 | COPY16);

  // (2) Create 4 separate palettes, each with empty index 0 and a different colour at index 1.
  BG_PALETTE[0 * 16 + 1] = RGB5(31, 0, 0);     // Red
  BG_PALETTE[1 * 16 + 1] = RGB5(0, 31, 0);     // Green
  BG_PALETTE[2 * 16 + 1] = RGB5(0, 0, 31);     // Blue
  BG_PALETTE[3 * 16 + 1] = RGB5(16, 16, 16);   // Grey

  // (3) Create a map, 4 blocks of tiles, each using a different palette.
  u16* screen_block = SCREEN_BASE_BLOCK(SCR_BASE_BLOCK);
  for (int block_idx = 0; block_idx < 4; block_idx++) {
    for (int tile_idx = 0; tile_idx < 1024; tile_idx++) {
      screen_block[block_idx * 1024 + tile_idx] = CHAR_PALETTE(block_idx) | 0;
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
