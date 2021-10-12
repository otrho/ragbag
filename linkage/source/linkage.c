#include <stdlib.h>

#include <gba.h>

#include "white-all.h"

// Wacky XOR method for min/max.  Better than a ternary op?
#define min(a, b) ((b) ^ (((a) ^ (b)) & -((a) < (b))))
#define max(a, b) ((a) ^ (((a) ^ (b)) & -((a) < (b))))

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

#define TOTAL_PIECE_COUNT 1024

struct Piece {
  u16 dirs;
  u8 rot;
  u8 is_linked;
};

static struct Piece g_puzzle_pieces[TOTAL_PIECE_COUNT];

#define PUZZLE_WIDTH 32
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

static s32 g_links[TOTAL_PIECE_COUNT];
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
  u16 piece_tiles_idx = c_dirs_piece_map[PUZZLE_PIECE_AT(x, y)->dirs];

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

static OBJATTR g_shadow_cursor_attrs[4];

static void move_cursor(u16 x, u16 y) {
  x *= 16;
  y *= 16;

  g_shadow_cursor_attrs[0].attr0 = (g_shadow_cursor_attrs[0].attr0 & 0xff00) | OBJ_Y(y + 0);
  g_shadow_cursor_attrs[0].attr1 = (g_shadow_cursor_attrs[0].attr1 & 0xfe00) | OBJ_X(x + 0);
  g_shadow_cursor_attrs[1].attr0 = (g_shadow_cursor_attrs[1].attr0 & 0xff00) | OBJ_Y(y + 0);
  g_shadow_cursor_attrs[1].attr1 = (g_shadow_cursor_attrs[1].attr1 & 0xfe00) | OBJ_X(x + 8);
  g_shadow_cursor_attrs[2].attr0 = (g_shadow_cursor_attrs[2].attr0 & 0xff00) | OBJ_Y(y + 8);
  g_shadow_cursor_attrs[2].attr1 = (g_shadow_cursor_attrs[2].attr1 & 0xfe00) | OBJ_X(x + 0);
  g_shadow_cursor_attrs[3].attr0 = (g_shadow_cursor_attrs[3].attr0 & 0xff00) | OBJ_Y(y + 8);
  g_shadow_cursor_attrs[3].attr1 = (g_shadow_cursor_attrs[3].attr1 & 0xfe00) | OBJ_X(x + 8);
}

static void enable_cursor() {
  g_shadow_cursor_attrs[0].attr0 &= ~ATTR0_DISABLED;
  g_shadow_cursor_attrs[1].attr0 &= ~ATTR0_DISABLED;
  g_shadow_cursor_attrs[2].attr0 &= ~ATTR0_DISABLED;
  g_shadow_cursor_attrs[3].attr0 &= ~ATTR0_DISABLED;
}

//static void disable_cursor() {
//  g_shadow_cursor_attrs[0].attr0 |= ATTR0_DISABLED;
//  g_shadow_cursor_attrs[1].attr0 |= ATTR0_DISABLED;
//  g_shadow_cursor_attrs[2].attr0 |= ATTR0_DISABLED;
//  g_shadow_cursor_attrs[3].attr0 |= ATTR0_DISABLED;
//}

// Should be called only during v-blank.
static void draw_cursor() {
  // sizeof obj attr / 4 == num u32s * 4 tiles.
  CpuFastSet(g_shadow_cursor_attrs, OAM + 0, (sizeof (OBJATTR) / 4 * 4) | COPY32);
}

// -------------------------------------------------------------------------------------------------

static void init_puzzle_bg() {
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

static void init_cursor_sprite(u32 sprite_idx) {
  static const u32 cursor_sprites[32] = {
    // Top left.
    0x00111111,
    0x00000001,
    0x00000001,
    0x00000001,
    0x00000001,
    0x00000001,
    0x00000000,
    0x00000000,

    // Top right.
    0x11111100,
    0x10000000,
    0x10000000,
    0x10000000,
    0x10000000,
    0x10000000,
    0x00000000,
    0x00000000,

    // Bot left.
    0x00000000,
    0x00000000,
    0x00000001,
    0x00000001,
    0x00000001,
    0x00000001,
    0x00000001,
    0x00111111,

    // Bot right.
    0x00000000,
    0x00000000,
    0x10000000,
    0x10000000,
    0x10000000,
    0x10000000,
    0x10000000,
    0x11111100,
  };

  u32* cursor_sprite_ptr = ((u32*)OBJ_BASE_ADR) + (sprite_idx * 8);
  for (u32 row_idx = 0; row_idx < 32; row_idx++) {
    cursor_sprite_ptr[row_idx] = cursor_sprites[row_idx];
  }

  // Set a couple of colours in the first palette, black and red.
  OBJ_COLORS[0] = RGB5(0, 0, 0);
  OBJ_COLORS[1] = RGB5(31, 0, 0);

  // Set the attributes for cursor sprite.
  for (s32 tile_idx = 0; tile_idx < 4; tile_idx++) {
    g_shadow_cursor_attrs[tile_idx].attr0 = OBJ_Y(0) | ATTR0_DISABLED | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE;
    g_shadow_cursor_attrs[tile_idx].attr1 = OBJ_X(0) | ATTR1_SIZE_8;
    g_shadow_cursor_attrs[tile_idx].attr2 = OBJ_CHAR(sprite_idx + tile_idx) | ATTR2_PRIORITY(0) | ATTR2_PALETTE(0);
  }
}

// -------------------------------------------------------------------------------------------------

static void init_sprites() {
  // Skip tile 0, it produces artefacts.
  init_cursor_sprite(1);        // Cursor sprite at tiles 1..4.
}

// -------------------------------------------------------------------------------------------------

static void init_gfx() {
  VBlankIntrWait();

  // Set the video mode to 0, enable bg 1, enable sprites, 1d mapping for sprites.
  SetMode(MODE_0 | BG1_ENABLE | OBJ_ENABLE | OBJ_1D_MAP);

  init_puzzle_bg();
  init_sprites();
}

// -------------------------------------------------------------------------------------------------

#define SCREEN_MAX_WIDTH 15
#define SCREEN_MAX_HEIGHT 10

struct CursorScroll {
  // These are mutable state.
  s32 cursor_x, cursor_y;
  s32 cursor_screen_x, cursor_screen_y;
  s32 puzzle_scroll_x, puzzle_scroll_y;                 // In pieces, not pixels.

  // These are const.
  s32 puzzle_max_scroll_x, puzzle_max_scroll_y;
  s32 puzzle_width, puzzle_height;
  s32 origin_x, origin_y;
};

static void init_cursor_scroll(struct CursorScroll* cursor_scroll,
                        s32 width, s32 height, s32 origin_x, s32 origin_y) {
  cursor_scroll->cursor_x = 0; cursor_scroll->cursor_y = 0;
  cursor_scroll->cursor_screen_x = 0; cursor_scroll->cursor_screen_y = 0;
  cursor_scroll->puzzle_scroll_x = 0; cursor_scroll->puzzle_scroll_y = 0;
  cursor_scroll->puzzle_max_scroll_x = width - SCREEN_MAX_WIDTH;
  cursor_scroll->puzzle_max_scroll_y = height - SCREEN_MAX_HEIGHT;
  cursor_scroll->puzzle_width = width; cursor_scroll->puzzle_height = height;
  cursor_scroll->origin_x = origin_x; cursor_scroll->origin_y = origin_y;
}

static void centre_puzzle(struct CursorScroll* cursor_scroll) {
  // To put the origin in the centre, we want it at 15/2,10/2 == 7,5, but we need to keep it within
  // the bounds.
  s32 tmp_x = max(cursor_scroll->origin_x - 7, 0);
  cursor_scroll->puzzle_scroll_x = min(tmp_x, cursor_scroll->puzzle_max_scroll_x);
  s32 tmp_y = max(cursor_scroll->origin_y - 5, 0);
  cursor_scroll->puzzle_scroll_y = min(tmp_y, cursor_scroll->puzzle_max_scroll_y);

  // The cursor needs to be at the origin.
  cursor_scroll->cursor_x = cursor_scroll->origin_x;
  cursor_scroll->cursor_y = cursor_scroll->origin_y;
  cursor_scroll->cursor_screen_x = cursor_scroll->origin_x - cursor_scroll->puzzle_scroll_x;
  cursor_scroll->cursor_screen_y = cursor_scroll->origin_y - cursor_scroll->puzzle_scroll_y;
}

static void cursor_up(struct CursorScroll* cursor_scroll) {
  if (cursor_scroll->cursor_y > 0) {
    cursor_scroll->cursor_y--;
    if (cursor_scroll->cursor_screen_y > 0) {
      cursor_scroll->cursor_screen_y--;
    } else {
      cursor_scroll->puzzle_scroll_y--;
    }
  }
}

static void cursor_down(struct CursorScroll* cursor_scroll) {
  if (cursor_scroll->cursor_y < cursor_scroll->puzzle_height - 1) {
    cursor_scroll->cursor_y++;
    if (cursor_scroll->cursor_screen_y < SCREEN_MAX_HEIGHT - 1) {
      cursor_scroll->cursor_screen_y++;
    } else {
      cursor_scroll->puzzle_scroll_y++;
    }
  }
}

static void cursor_left(struct CursorScroll* cursor_scroll) {
  if (cursor_scroll->cursor_x > 0) {
    cursor_scroll->cursor_x--;
    if (cursor_scroll->cursor_screen_x > 0) {
      cursor_scroll->cursor_screen_x--;
    } else {
      cursor_scroll->puzzle_scroll_x--;
    }
  }
}

static void cursor_right(struct CursorScroll* cursor_scroll) {
  if (cursor_scroll->cursor_x < cursor_scroll->puzzle_width - 1) {
    cursor_scroll->cursor_x++;
    if (cursor_scroll->cursor_screen_x < SCREEN_MAX_WIDTH - 1) {
      cursor_scroll->cursor_screen_x++;
    } else {
      cursor_scroll->puzzle_scroll_x++;
    }
  }
}

static void update_screen_cursor_scroll(struct CursorScroll* cursor_scroll) {
      REG_BG1HOFS = (u16)cursor_scroll->puzzle_scroll_x * 16;   // Pixels...
      REG_BG1VOFS = (u16)cursor_scroll->puzzle_scroll_y * 16;
      move_cursor((u16)cursor_scroll->cursor_screen_x,
                  (u16)cursor_scroll->cursor_screen_y);
      draw_cursor();
}

// -------------------------------------------------------------------------------------------------

static void puzzle_loop(struct CursorScroll* cursor_scroll) {

  centre_puzzle(cursor_scroll);

  VBlankIntrWait();
  enable_cursor();
  update_screen_cursor_scroll(cursor_scroll);

  while (1) {
    scanKeys();
    u16 keys_up = keysUp();

    if (keys_up & KEY_START) {
      break;
    }

    if (keys_up & KEY_UP) {
      cursor_up(cursor_scroll);
    }
    if (keys_up & KEY_DOWN) {
      cursor_down(cursor_scroll);
    }
    if (keys_up & KEY_LEFT) {
      cursor_left(cursor_scroll);
    }
    if (keys_up & KEY_RIGHT) {
      cursor_right(cursor_scroll);
    }

    VBlankIntrWait();
    if (keys_up & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
      update_screen_cursor_scroll(cursor_scroll);
    }
  }
}

// -------------------------------------------------------------------------------------------------

int main() {
  irqInit();
  irqEnable(IRQ_VBLANK);
  REG_IME = 1;

  // Using timer 2 to seed the PRNG.  The count between start and when the user first presses 'A'.
  REG_TM2CNT_H |= TIMER_START;

  // Load our backrounds and sprites into VRAM.
  init_gfx();

  while (1) {
    VBlankIntrWait();

    scanKeys();
    u16 keys_up = keysUp();

    if (keys_up & KEY_A) {
      break;
    }
  }

  srandom(REG_TM2CNT);

  // Small = 1 screen = 15x10
  // Medium = middle = 24x24
  // Large = entire bg = 32x32
  s32 width = 32;
  s32 height = 32;
  s32 origin_x;
  s32 origin_y;
  generate_puzzle(width, height, &origin_x, &origin_y);

  VBlankIntrWait();
  for (s32 y = 0; y < height; y++) {
    for (s32 x = 0; x < width; x++) {
      update_puzzle_board(x, y);
    }
  }

  struct CursorScroll cursor_scroll;
  init_cursor_scroll(&cursor_scroll, width, height, origin_x, origin_y);
  puzzle_loop(&cursor_scroll);

  Stop();
  return 0;
}

// -------------------------------------------------------------------------------------------------
