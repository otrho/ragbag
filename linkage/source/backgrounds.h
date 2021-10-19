#pragma once

// Use BG1 for the puzzle tiles.  Load the tile data to the first character block, use the last
// screen block for the tilemap.

#define PUZZLE_BG_IDX 1
#define CHAR_BASE_IDX 0
#define MAP_BASE_IDX 28

#define LIT_PALETTE 0
#define DULL_PALETTE 1

void init_puzzle_bg();
