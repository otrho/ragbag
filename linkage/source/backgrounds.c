#include <gba.h>

#include "backgrounds.h"
#include "white-all.h"

// -------------------------------------------------------------------------------------------------

void init_puzzle_bg() {
  // Use the largest 512x512 size for the puzzle bg.
  BGCTRL[PUZZLE_BG_IDX] = CHAR_BASE(CHAR_BASE_IDX) | SCREEN_BASE(MAP_BASE_IDX) | BG_SIZE_3;

  // Load the tile image data.
  CpuFastSet(white_allTiles, CHAR_BASE_ADR(CHAR_BASE_IDX), 512 | COPY32);
  BG_PALETTE[0] = RGB5(0x0, 0x0, 0x0);
  BG_PALETTE[1] = RGB5(0x0f, 0x0f, 0x0f);

  // Make sure scroll is at the top left.
  REG_BG1HOFS = 0;
  REG_BG1VOFS = 0;
}

// -------------------------------------------------------------------------------------------------
