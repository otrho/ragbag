#include <stdlib.h>

#include <gba.h>

#include "white-all.h"

// -------------------------------------------------------------------------------------------------
// Use BG1 for the puzzle tiles.  Load the tile data to the first character block, use the last
// screen block for the tilemap.

#define PUZZLE_BG_IDX 1
#define CHAR_BASE_IDX 0
#define MAP_BASE_IDX 28

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
// |                                   | l | rot   | dirs          |
// +-----------------------------------+---+-------+---------------+
//  * l = Is piece linked?
//  * We assume that the dirs are in the bottom of the value so we can easily bit-or them in or out.

static u16 g_puzzle_pieces[1024];

#define PUZZLE_WIDTH 32
#define PUZZLE_DIRS_AT(x, y) (g_puzzle_pieces[(y) * PUZZLE_WIDTH + (x)] & 0xf)

enum Direction {
  kNorth = 1,
  kEast = 2,
  kSouth = 4,
  kWest = 8,
};

void move_in_dir(s32* x, s32* y, enum Direction dir) {
  switch (dir) {
    case kNorth: (*y)--; break;
    case kEast: (*x)++; break;
    case kSouth: (*y)++; break;
    case kWest: (*x)--; break;
  }
}

enum Direction opposite_dir(enum Direction dir) {
  switch (dir) {
    case kNorth: return kSouth;
    case kEast: return kWest;
    case kSouth: return kNorth;
    case kWest: return kEast;
  }
  return 0;
}

// -------------------------------------------------------------------------------------------------

//static void set_piece(s32 x, s32 y,
//                      s32 directions,
//                      s32 rotation, s32 is_linked) {
//  s32 piece = (directions << 0) | (rotation << 4) | (is_linked << 6);
//  g_puzzle_pieces[y * 32 + x] = (u16)(piece & 0xffff);
//}

// -------------------------------------------------------------------------------------------------
// A 4KB buffer for tracking where the current ends of the expansion are.  Can push to the top, pop
// off the top or bottom.

static s32 g_links[1024];
static s32 g_links_top;
static s32 g_links_bot;

void links_init() {
  g_links_top = 0;
  g_links_bot = 0;
}

s32 links_empty() {
  return g_links_top == g_links_bot;
}

void links_push(s32 x, s32 y) {
  g_links[g_links_top++] = (x << 16) | y;
}

void links_pop_back() {
  g_links_top--;
}

void links_pop_front() {
  g_links_bot++;
}

void links_back(s32* x, s32* y) {
  s32 val = g_links[g_links_top - 1];
  *x = (val >> 16) & 0xffff;
  *y = val & 0xffff;
}

void links_front(s32* x, s32* y) {
  s32 val = g_links[g_links_bot];
  *x = (val >> 16) & 0xffff;
  *y = val & 0xffff;
}

// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

s32 extend(s32 width, s32 height, s32 src_x, s32 src_y, enum Direction dir, s32* dst_x, s32* dst_y) {
  *dst_x = src_x;
  *dst_y = src_y;
  move_in_dir(dst_x, dst_y, dir);
  if (*dst_x < 0 || *dst_x >= width || *dst_y < 0 || *dst_y >= height) {
    // Out of bounds.
    return 0;
  }
  if (PUZZLE_DIRS_AT(*dst_x, *dst_y) != 0) {
    // Piece in that direction is not empty.
    return 0;
  }

  // Extend the link.
  g_puzzle_pieces[src_y * PUZZLE_WIDTH + src_x] |= dir;
  g_puzzle_pieces[*dst_y * PUZZLE_WIDTH + *dst_x] |= opposite_dir(dir);

  return 1;
}

// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
// Puzzle generation is just a space filling algorithm using random source and direction.
//
// Initialises g_puzzle_pieces.

void generate_puzzle(s32 width, s32 height, s32* origin_x, s32* origin_y) {
  // Clear the puzzle.
  s32 null = 0;
  CpuFastSet(&null, g_puzzle_pieces, (1024 / 2) | FILL | COPY32);

  // Set the origin to somewhere random, at least 2 pieces from the edge.
  *origin_x = (random() % (width - 4)) + 2;
  *origin_y = (random() % (height - 4)) + 2;

  // Initialise the links stack.
  links_init();

  // Push out in all 4 directions from the origin.
  s32 next_x = 0, next_y = 0;
  extend(width, height, *origin_x, *origin_y, kNorth, &next_x, &next_y);
  links_push(*origin_x + 0, *origin_y - 1);
  extend(width, height, *origin_x, *origin_y, kEast, &next_x, &next_y);
  links_push(*origin_x - 1, *origin_y + 0);
  extend(width, height, *origin_x, *origin_y, kSouth, &next_x, &next_y);
  links_push(*origin_x + 0, *origin_y + 1);
  extend(width, height, *origin_x, *origin_y, kWest, &next_x, &next_y);
  links_push(*origin_x + 1, *origin_y + 0);

  // Generate the map.
  while (!links_empty()) {
    // Grab the next link.  50/50 split as to whether we use the newest or oldest.
    s32 use_back = (random() % 2) == 0;
    s32 x, y;
    if (use_back) {
      links_back(&x, &y);
    } else {
      links_front(&x, &y);
    }

    // __builtin_popcount() is a GNU CC builtin to count the bits set in an int.
#define num_links __builtin_popcount

    // We want a maximum of 3 branches.
    s32 extended = 0;
    if (num_links(PUZZLE_DIRS_AT(x, y)) < 3) {
      // Keep looping until we find extend successfully.
      for (s32 attempted_dirs = 0; num_links(attempted_dirs) < 4; ) {
        enum Direction next_dir = (kNorth << (random() % 4));
        if ((attempted_dirs & next_dir) != 0) {
          // We've already tried this way.  Try again.
          continue;
        }
        attempted_dirs |= next_dir;

        // Can we go in that direction?
        if (!extend(width, height, x, y, next_dir, &next_x, &next_y)) {
          // Can't go that way.  Try again.
          continue;
        }
        links_push(next_x, next_y);
        extended = 1;
        break;
      }
    }

    if (!extended) {
      // We failed to extend this link; remove it from the list.
      if (use_back) {
        links_pop_back();
      } else {
        links_pop_front();
      }
    }
  }

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
   0,   // 15 = north|east|south|west, is piece at 0,0 = 0
};

// NOTE: We write directly to VRAM and should only be attempted during v-blank.

// The entire 512x512px backround is made up of 64x64 tiles, each being 8x8px.
// We need 4 screen blocks, which are only 2KB each, or 32x32 = 1024 tiles, 16bit entries = 2KB.
// The screen blocks are:
//   0 | 1
//   --+--
//   2 | 3
// so we need to work that out when determining how to write a puzzle piece.
//
// x, y below are *piece* coords, each piece is 2x2 tiles.

static void update_puzzle_board(s32 x, s32 y) {
  u16 piece_tiles_idx = c_dirs_piece_map[PUZZLE_DIRS_AT(x, y)];

  // I'm hoping the compiler makes this much more efficient.  I should check the ASM.
  s32 scr_blk_idx = (y * 2) / NUM_Y_BG_TILES * 2 + (x * 2) / NUM_Y_BG_TILES;
  u16* tilemap_block = SCREEN_BASE_BLOCK(MAP_BASE_IDX + scr_blk_idx);
  s32 tilemap_idx = ((y * 2) % NUM_Y_BG_TILES) * NUM_Y_BG_TILES + ((x * 2) % NUM_Y_BG_TILES);

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
  CpuFastSet(white_allPal, BG_PALETTE, 8 | COPY16);

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

  // Using timer 2 to seed the PRNG.  The count between start and when the user first presses 'A'.
  REG_TM2CNT_H |= TIMER_START;

  init_gfx();

  // Small = 1 screen = 15x10
  // Medium = middle = 24x24
  // Large = entire bg = 32x32
  s32 width = 32;
  s32 height = 32;

  u16 scroll_x = 0;
  u16 scroll_y = 0;

  s32 initialised = 0;
  while (1) {
    VBlankIntrWait();

    scanKeys();
    u16 keys_up = keysUp();

    if (keys_up & KEY_A) {
      if (!initialised) {
        initialised = 1;
        srandom(REG_TM2CNT);
        s32 origin_x;
        s32 origin_y;
        generate_puzzle(width, height, &origin_x, &origin_y);
        VBlankIntrWait();
        for (s32 y = 0; y < height; y++) {
          for (s32 x = 0; x < width; x++) {
            update_puzzle_board(x, y);
          }
        }
      }
    }

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

// -------------------------------------------------------------------------------------------------
