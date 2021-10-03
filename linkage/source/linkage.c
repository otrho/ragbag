#include <gba.h>

#include "white-all.h"

// -------------------------------------------------------------------------------------------------
// Use BG1 for the puzzle tiles.  Load the tile data to the first character block, use the last
// screen block for the tilemap.

#define PUZZLE_BG_IDX 1
#define CHAR_BASE_IDX 0
#define MAP_BASE_IDX 31

#define NUM_Y_BG_TILES 32               // 32 8x8 tiles in bg quadrant row.
#define NUM_Y_MAP_TILES 8               // 8 8x8 tiles in our tile set row.

// -------------------------------------------------------------------------------------------------
// Puzzle board is always 32x32 (1024) pieces, even if the puzzle itself is smaller than that.
//
// Each piece is a u16.
//
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// | f | e | d | c | b | a | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// |                   | l | rot   | current dirs  | puzzle dirs   |
// +-------------------+---+-------+---------------+---------------+
//  * l = Is piece linked?

static u16 g_puzzle_pieces[1024];

#define GET_PUZZLE_DIRS(p) ((p) & 0xf)

enum Direction {
  kNorth = 1,
  kEast = 2,
  kSouth = 4,
  kWest = 8,
};

// -------------------------------------------------------------------------------------------------

static void set_piece(s32 x, s32 y,
                      s32 puzzle_dirs, s32 current_dirs,
                      s32 rotation, s32 is_linked) {
  s32 piece = (puzzle_dirs << 0) | (current_dirs << 4) | (rotation << 8) | (is_linked << 10);
  g_puzzle_pieces[y * 32 + x] = (u16)(piece & 0xffff);
}

// -------------------------------------------------------------------------------------------------
// The tiles in the map are loaded from a 8x8 grid of tiles, representing a 4x4 grid of pieces.
//
//   0    2    4    6
// 0   |    |         |
//   --+--  |  --+--  +--
//     |    |    |
// 2             |
//     o  --+----+    +--
//     |         |    |
// 4       \ /   |
//   --o    x  --+----+
//         / \        |
// 6   |         |    |
//     o    o--  +----+
//               |
//
// The 4-bit direction in the piece maps to one of these pieces, each of which has 2x2 tiles.

static const u16 c_dirs_piece_map[16] = {
  34,   // 0 = no dirs,                is piece at 4,2 = 34
  48,   // 1 = north,                  is piece at 6,0 = 48
  50,   // 2 = east,                   is piece at 6,2 = 50
   6,   // 3 = north|east,             is piece at 0,6 = 6
  16,   // 4 = south,                  is piece at 2,0 = 16
   2,   // 5 = north|south,            is piece at 0,2 = 2
  22,   // 6 = east|south,             is piece at 2,6 = 22
  52,   // 7 = north|east|south,       is piece at 6,4 = 52
  32,   // 8 = west,                   is piece at 4,0 = 32
  54,   // 9 = north|west,             is piece at 6,6 = 54
  18,   // 10 = east|west,             is piece at 2,2 = 18
  36,   // 11 = north|east|west,       is piece at 4,4 = 36
  38,   // 12 = south|west,            is piece at 4,6 = 38
  20,   // 13 = north|south|west,      is piece at 2,4 = 20
   4,   // 14 = east|south|west,       is piece at 0,4 = 4
   0,   // 14 = north|east|south|west, is piece at 0,0 = 0
};

// NOTE: We write directly to VRAM and should only be attempted during v-blank.

static void update_puzzle_board(s32 x, s32 y) {
  u16 piece = g_puzzle_pieces[y * 32 + x];
  u16 piece_tiles_idx = c_dirs_piece_map[GET_PUZZLE_DIRS(piece)];

  u16* tilemap_block = SCREEN_BASE_BLOCK(MAP_BASE_IDX);
  // XXX This doesn't take the 4 quadrants into account yet...
  s32 tilemap_idx = y * 2 * NUM_Y_BG_TILES + x * 2;

  tilemap_block[tilemap_idx + 0] = piece_tiles_idx + 0;
  tilemap_block[tilemap_idx + 1] = piece_tiles_idx + 1;
  tilemap_block[tilemap_idx + NUM_Y_BG_TILES + 0] = piece_tiles_idx + NUM_Y_MAP_TILES + 0;
  tilemap_block[tilemap_idx + NUM_Y_BG_TILES + 1] = piece_tiles_idx + NUM_Y_MAP_TILES + 1;
}

// -------------------------------------------------------------------------------------------------

static void init_puzzle_bg() {
  // Use the largest 512x512 size for the puzzle bg.
  BGCTRL[PUZZLE_BG_IDX] = CHAR_BASE(CHAR_BASE_IDX) | SCREEN_BASE(MAP_BASE_IDX) | BG_SIZE_3;

  // Load the tile image data.
  CpuFastSet(white_allTiles, CHAR_BASE_ADR(CHAR_BASE_IDX), 512 | COPY32);
  CpuFastSet(white_allPal, BG_PALETTE, 16 | COPY16);

  // Make sure scroll is at the top left.
  REG_BG1HOFS = 0;
  REG_BG1VOFS = 0;
}

// -------------------------------------------------------------------------------------------------

static void init_gfx() {
  VBlankIntrWait();

  // Set the video mode to 0, enable bg 1, enable objects.
  SetMode(MODE_0 | BG1_ENABLE | OBJ_ENABLE);

  init_puzzle_bg();
}

// -------------------------------------------------------------------------------------------------

int main() {
  irqInit();
  irqEnable(IRQ_VBLANK);
  REG_IME = 1;

  init_gfx();

  // Temporary testing of the puzzle pieces code.
  s32 dirs = kEast | kSouth;                    //   +--
  set_piece(0, 0, dirs, dirs, 0, 0);            //   |
  dirs = kWest | kSouth;                        // --+
  set_piece(1, 0, dirs, dirs, 0, 0);            //   |
  dirs = kEast | kNorth;                        //   |
  set_piece(0, 1, dirs, dirs, 0, 0);            //   +--
  dirs = kWest | kNorth;                        //   |
  set_piece(1, 1, dirs, dirs, 0, 0);            // --+

  update_puzzle_board(0, 0);
  update_puzzle_board(1, 0);
  update_puzzle_board(0, 1);
  update_puzzle_board(1, 1);

  set_piece(4, 4, 0, 0, 0, 0);
  update_puzzle_board(4, 4);

  while (1) {
    VBlankIntrWait();

    scanKeys();
    u16 keys_up = keysUp();

    if (keys_up & KEY_START) {
      break;
    }

  }

  Stop();
  return 0;
}

// -------------------------------------------------------------------------------------------------
