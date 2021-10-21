#include <stdlib.h>
#include <gba.h>

#include "puzzle.h"
#include "backgrounds.h"
#include "stack.h"

// -------------------------------------------------------------------------------------------------
// Puzzle board is always 32x32 (1024) pieces, even if the puzzle itself is smaller than that.

#define TOTAL_PIECE_COUNT 1024

struct Piece {
  u16 dirs;
  u8 rot;
  u8 is_linked;
};

static struct Piece g_puzzle_pieces[TOTAL_PIECE_COUNT];

#define PUZZLE_WIDTH 32
#define PUZZLE_HEIGHT 32
#define PUZZLE_PIECE_AT(x, y) (g_puzzle_pieces + (y) * PUZZLE_WIDTH + (x))

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
// A 4KB buffer for tracking where the current ends of the expansion are.  Can push to the top, pop
// off the top or bottom.

void links_back(s32* x, s32* y) {
  s32 val = stack_top();
  *x = (val >> 16) & 0xffff;
  *y = val & 0xffff;
}

void links_front(s32* x, s32* y) {
  s32 val = stack_bot();
  *x = (val >> 16) & 0xffff;
  *y = val & 0xffff;
}

void links_push(s32 x, s32 y) {
  stack_push((x << 16) | y);
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

  struct Piece* dst_piece = PUZZLE_PIECE_AT(*dst_x, *dst_y);
  if (dst_piece->dirs != 0) {
    // Piece in that direction is not empty.
    return 0;
  }

  // Extend the link.
  PUZZLE_PIECE_AT(src_x, src_y)->dirs |= dir;
  dst_piece->dirs |= opposite_dir(dir);

  return 1;
}

// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
// Puzzle generation is just a space filling algorithm using random source and direction.
//
// Initialises g_puzzle_pieces.

void generate_puzzle(s32 width, s32 height, s32* origin_x, s32* origin_y) {
  // Clear the puzzle.
  s32 null = 0;
  CpuFastSet(&null, g_puzzle_pieces, (sizeof (struct Piece) * TOTAL_PIECE_COUNT / 4) | FILL | COPY32);

  // Set the origin to somewhere random, at least 2 pieces from the edge.
  *origin_x = (random() % (width - 4)) + 2;
  *origin_y = (random() % (height - 4)) + 2;

  // Initialise the links stack.
  stack_init();

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
  while (!stack_empty()) {
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
    if (num_links(PUZZLE_PIECE_AT(x, y)->dirs) < 3) {
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
        stack_pop_top();
      } else {
        stack_pop_bot();
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

#define NUM_Y_BG_TILES 32               // 32 8x8 tiles in bg quadrant row.
#define NUM_Y_MAP_TILES 8               // 8 8x8 tiles in our tile set row.

void update_puzzle_board(s32 x, s32 y, u16 pal_idx) {
  u16 piece_tiles_idx = CHAR_PALETTE(pal_idx) | c_dirs_piece_map[PUZZLE_PIECE_AT(x, y)->dirs];

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
// dir must be +1 (clockwise) or -1 (anticlockwise).

static const u16 c_dirs_clockwise_map[16] = {
  0,                            // 0 = no dirs
  kEast,                        // 1 = north -> east
  kSouth,                       // 2 = east -> south
  kEast|kSouth,                 // 3 = north|east -> east|south
  kWest,                        // 4 = south -> west
  kEast|kWest,                  // 5 = north|south -> east|west
  kSouth|kWest,                 // 6 = east|south -> south|west
  kEast|kSouth|kWest,           // 7 = north|east|south -> east|south|west
  kNorth,                       // 8 = west -> north
  kNorth|kEast,                 // 9 = north|west -> north|east
  kNorth|kSouth,                // 10 = east|west -> north|south
  kNorth|kEast|kSouth,          // 11 = north|east|west -> north|east|south
  kNorth|kWest,                 // 12 = south|west -> north|west
  kNorth|kEast|kWest,           // 13 = north|south|west -> north|east|west
  kNorth|kSouth|kWest,          // 14 = east|south|west -> north|south|west
  kNorth|kEast|kSouth|kWest,    // 15 = north|east|south|west -> no rotation
};

static const u16 c_dirs_anticlockwise_map[16] = {
  0,                            // 0 = no dirs
  kWest,                        // 1 = north -> west
  kNorth,                       // 2 = east -> north
  kNorth|kWest,                 // 3 = north|east -> north|west
  kEast,                        // 4 = south -> east
  kEast|kWest,                  // 5 = north|south -> east|west
  kNorth|kEast,                 // 6 = east|south -> north|east
  kNorth|kEast|kWest,           // 7 = north|east|south -> north|east|west
  kSouth,                       // 8 = west -> south
  kSouth|kWest,                 // 9 = north|west -> south|west
  kNorth|kSouth,                // 10 = east|west -> north|south
  kNorth|kSouth|kWest,          // 11 = north|east|west -> north|south|west
  kEast|kSouth,                 // 12 = south|west -> east|south
  kEast|kSouth|kWest,           // 13 = north|south|west -> east|south|west
  kNorth|kEast|kSouth,          // 14 = east|south|west -> north|east|south
  kNorth|kEast|kSouth|kWest,    // 15 = north|east|south|west -> no rotation
};

void rotate_piece(s32 x, s32 y, s32 dir) {
  struct Piece* piece = PUZZLE_PIECE_AT(x, y);

  if (piece->dirs == (kNorth | kEast | kSouth | kWest)) {
    // Do nothing, can't rotate the cross.
    return;
  }

  if (piece->dirs == (kNorth | kSouth) || piece->dirs == (kEast | kWest)) {
    // Straight line is symmetrical and can toggle rotation between 0 and 1.
    piece->rot ^= 1;
  } else {
    piece->rot = (piece->rot + dir) & 3;
  }

  if (dir == 1) {
    piece->dirs = c_dirs_clockwise_map[piece->dirs];
  } else {
    piece->dirs = c_dirs_anticlockwise_map[piece->dirs];
  }
}

// -------------------------------------------------------------------------------------------------

s32 puzzle_piece_is_lit(s32 x, s32 y) {
  return (s32)(PUZZLE_PIECE_AT(x, y)->is_linked);
}

// -------------------------------------------------------------------------------------------------
// This is dumb, optimise it later.

#define PUSH_DIR(x, y, d) stack_push(((x) << 20) | (((y) & 0xfff) << 8) | d)

void update_next_link() {
  s32 value = stack_top();
  stack_pop_top();
  s32 x = ((value >> 20) & 0xfff), y = ((value >> 8) & 0xfff), from_d = opposite_dir(value & 0xff);

  if (x >= PUZZLE_WIDTH || y >= PUZZLE_HEIGHT) {
    return;
  }

  struct Piece* piece = PUZZLE_PIECE_AT(x, y);
  if (piece->is_linked == 1 || (piece->dirs & from_d) == 0) {
    return;
  }
  piece->is_linked = 1;

  if (from_d != kNorth && (piece->dirs & kNorth) != 0) {
    PUSH_DIR(x, y - 1, kNorth);
  }
  if (from_d != kEast && (piece->dirs & kEast) != 0) {
    PUSH_DIR(x + 1, y, kEast);
  }
  if (from_d != kSouth && (piece->dirs & kSouth) != 0) {
    PUSH_DIR(x, y + 1, kSouth);
  }
  if (from_d != kWest && (piece->dirs & kWest) != 0) {
    PUSH_DIR(x - 1, y, kWest);
  }
}

void render_lit_tiles(s32 origin_x, s32 origin_y) {
  for (s32 idx = 0; idx < TOTAL_PIECE_COUNT; idx++) {
    g_puzzle_pieces[idx].is_linked = 0;
  }

  PUZZLE_PIECE_AT(origin_x, origin_y)->is_linked = 1;

  stack_init();
  PUSH_DIR(origin_x + 0, origin_y - 1, kNorth);
  PUSH_DIR(origin_x + 1, origin_y + 0, kEast);
  PUSH_DIR(origin_x + 0, origin_y + 1, kSouth);
  PUSH_DIR(origin_x - 1, origin_y + 0, kWest);

  while (!stack_empty()) {
    update_next_link();
  }
}

// -------------------------------------------------------------------------------------------------
