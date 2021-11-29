#include <gba.h>

#include "backgrounds.h"
#include "pieces.h"

// -------------------------------------------------------------------------------------------------

void init_puzzle_bg() {
  // Use the largest 512x512 size for the puzzle bg.
  BGCTRL[PUZZLE_BG_IDX] = CHAR_BASE(CHAR_BASE_IDX) | SCREEN_BASE(MAP_BASE_IDX) | BG_SIZE_3;

  // Load the tile image data.
  CpuFastSet(piecesTiles, CHAR_BASE_ADR(CHAR_BASE_IDX), 512 | COPY32);

  // Copy the glowy palette in and manually create our dull palette, which is all black except the
  // last colour.  These correspond to DULL_PALETTE and LIT_PALETTE in backgrounds.h.
  CpuFastSet(piecesPal, BG_PALETTE, 8 | COPY32);
  s32 black = 0;
  CpuFastSet(&black, BG_PALETTE + 16, 8 | FILL | COPY32);
  BG_PALETTE[31] = RGB5(0x1f, 0, 0x1f);

  // Make sure scroll is at the top left.
  REG_BG1HOFS = 0;
  REG_BG1VOFS = 0;
}

// -------------------------------------------------------------------------------------------------

void palette_flash_screen(s32 total_frames) {
  // We have two palettes, 16 entries of 16 RGB values each.  Make a copy first.
  u16 orig_palettes[16 * 2];
  CpuFastSet(BG_PALETTE, orig_palettes, 16 | COPY32);

  s32 ramp_frames = total_frames / 2;

  // For each entry we scale up and back again in total_frames frames.  Each entry is adjusted by a
  // fixed amount each frame.  We can precalculate the adjustment for each RGB value in a LUT.
  u16 adjustments[16 * 2];
  for (s32 idx = 0; idx < 16 * 2; idx++) {
    u16 o = orig_palettes[idx];
    u16 rd = (0x1f - ((o >> 10) & 0x1f)) / ramp_frames;
    u16 gr = (0x1f - ((o >>  5) & 0x1f)) / ramp_frames;
    u16 bl = (0x1f - ((o >>  0) & 0x1f)) / ramp_frames;
    adjustments[idx] = (rd << 10) | (gr << 5) | bl;
  }

  for (s32 frame = 0; frame < ramp_frames; frame++) {
    VBlankIntrWait();
    for (s32 idx = 0; idx < 16 * 2; idx++) {
      BG_PALETTE[idx] += adjustments[idx];
    }
  }
  for (s32 frame = 0; frame < ramp_frames; frame++) {
    VBlankIntrWait();
    for (s32 idx = 0; idx < 16 * 2; idx++) {
      BG_PALETTE[idx] -= adjustments[idx];
    }
  }

  // Restore the original palette exactly.
  CpuFastSet(orig_palettes, BG_PALETTE, 16 | COPY32);
}

// -------------------------------------------------------------------------------------------------
