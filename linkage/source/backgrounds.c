#include <gba.h>

#include "backgrounds.h"
#include "pieces.h"

// -------------------------------------------------------------------------------------------------

void init_puzzle_bg() {
  // Use the largest 512x512 size for the puzzle bg.
  BGCTRL[PUZZLE_BG_IDX] = CHAR_BASE(CHAR_BASE_IDX) | SCREEN_BASE(MAP_BASE_IDX) | BG_SIZE_3;

  // Load the tile image data.
  CpuFastSet(piecesTiles, CHAR_BASE_ADR(CHAR_BASE_IDX), 512 | COPY32);
  CpuFastSet(piecesPal, BG_PALETTE, 8 | COPY32);

  // Make sure scroll is at the top left.
  REG_BG1HOFS = 0;
  REG_BG1VOFS = 0;
}

// -------------------------------------------------------------------------------------------------
