#include <gba.h>

#include "sprites.h"

// -------------------------------------------------------------------------------------------------
// REMEMBER: Maximum 128 sprites, only 32 of them are affine.  Every 4 attribs structs has 1 affine
// struct interspersed through them.

// We have 4 cursor sprites and 1 spark sprite.
#define TOTAL_SPRITES 5

// Attributes indices.
#define CURSOR_ATTR_BASE 0
#define SPARK_ATTR_BASE 4

// Palette indices.  16 colours in a palette.
#define CURSOR_PALETTE_IDX 0
#define SPARK_PALETTE_IDX 1

#define CURSOR_PALETTE_BASE (CURSOR_PALETTE_IDX * 16)
#define SPARK_PALETTE_BASE (SPARK_PALETTE_IDX * 16)

// Pixel data indices.  We skip the first one at 0 since it seems to cause display artefacts.
#define CURSOR_DATA_BASE 1
#define SPARK_DATA_BASE 5

static OBJATTR g_shadow_attrs[TOTAL_SPRITES];
//static OBJAFFINE* g_shadow_affines = (OBJAFFINE*)g_shadow_attrs;

// -------------------------------------------------------------------------------------------------
// Should be called only during v-blank.

void draw_puzzle_sprites() {
  CpuFastSet(g_shadow_attrs, OAM, (sizeof (OBJATTR) / 4 * TOTAL_SPRITES) | COPY32);
}

// -------------------------------------------------------------------------------------------------

void move_sprite(s32 idx, u16 x, u16 y) {
  // Would it be cheaper to keep the other attribs stored pre-masked, add the pos and copy straight
  // in, rather than reading from the shadow like this?
  g_shadow_attrs[idx].attr0 = (g_shadow_attrs[idx].attr0 & 0xff00) | OBJ_Y(y + 0);
  g_shadow_attrs[idx].attr1 = (g_shadow_attrs[idx].attr1 & 0xfe00) | OBJ_X(x + 0);
}

// -------------------------------------------------------------------------------------------------
// Cursor manipulation.

// x,y are puzzle coords.
void move_cursor(u16 x, u16 y) {
  x *= 16;
  y *= 16;

  move_sprite(CURSOR_ATTR_BASE + 0, x + 0, y + 0);
  move_sprite(CURSOR_ATTR_BASE + 1, x + 8, y + 0);
  move_sprite(CURSOR_ATTR_BASE + 2, x + 0, y + 8);
  move_sprite(CURSOR_ATTR_BASE + 3, x + 8, y + 8);
}

void enable_cursor() {
  g_shadow_attrs[CURSOR_ATTR_BASE + 0].attr0 &= ~ATTR0_DISABLED;
  g_shadow_attrs[CURSOR_ATTR_BASE + 1].attr0 &= ~ATTR0_DISABLED;
  g_shadow_attrs[CURSOR_ATTR_BASE + 2].attr0 &= ~ATTR0_DISABLED;
  g_shadow_attrs[CURSOR_ATTR_BASE + 3].attr0 &= ~ATTR0_DISABLED;
}

void disable_cursor() {
  g_shadow_attrs[CURSOR_ATTR_BASE + 0].attr0 |= ATTR0_DISABLED;
  g_shadow_attrs[CURSOR_ATTR_BASE + 1].attr0 |= ATTR0_DISABLED;
  g_shadow_attrs[CURSOR_ATTR_BASE + 2].attr0 |= ATTR0_DISABLED;
  g_shadow_attrs[CURSOR_ATTR_BASE + 3].attr0 |= ATTR0_DISABLED;
}

// -------------------------------------------------------------------------------------------------

static void init_cursor_sprite() {
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

  u32* cursor_sprite_ptr = ((u32*)OBJ_BASE_ADR) + (CURSOR_DATA_BASE * 8);
  for (u32 row_idx = 0; row_idx < 32; row_idx++) {
    cursor_sprite_ptr[row_idx] = cursor_sprites[row_idx];
  }

  // Set a couple of colours in the first palette, black and red.
  OBJ_COLORS[CURSOR_PALETTE_BASE + 0] = RGB5(0, 0, 0);
  OBJ_COLORS[CURSOR_PALETTE_BASE + 1] = RGB5(31, 0, 0);

  // Set the attributes for cursor sprite.
  for (s32 tile_idx = 0; tile_idx < 4; tile_idx++) {
    g_shadow_attrs[CURSOR_ATTR_BASE + tile_idx].attr0 = OBJ_Y(0) | ATTR0_DISABLED | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE;
    g_shadow_attrs[CURSOR_ATTR_BASE + tile_idx].attr1 = OBJ_X(0) | ATTR1_SIZE_8;
    g_shadow_attrs[CURSOR_ATTR_BASE + tile_idx].attr2 = OBJ_CHAR(CURSOR_DATA_BASE + tile_idx) | ATTR2_PRIORITY(0) | ATTR2_PALETTE(CURSOR_PALETTE_IDX);
  }
}

// -------------------------------------------------------------------------------------------------

static void init_spark_sprite() {
  static const u32 spark_sprite[8] = {
    0x00000000,
    0x00555500,
    0x05566550,
    0x05677650,
    0x05677650,
    0x05566550,
    0x00555500,
    0x00000000,
  };

  u32* spark_sprite_ptr = ((u32*)OBJ_BASE_ADR) + (SPARK_DATA_BASE * 8);
  for (u32 row_idx = 0; row_idx < 8; row_idx++) {
    spark_sprite_ptr[row_idx] = spark_sprite[row_idx];
  }

  // Set colours in the second palette.
  OBJ_COLORS[SPARK_PALETTE_BASE + 0] = RGB5(0, 0, 0);
  OBJ_COLORS[SPARK_PALETTE_BASE + 1] = RGB5(0, 0, 0);
  OBJ_COLORS[SPARK_PALETTE_BASE + 2] = RGB5(0, 0, 0);
  OBJ_COLORS[SPARK_PALETTE_BASE + 3] = RGB5(0, 0, 0);
  OBJ_COLORS[SPARK_PALETTE_BASE + 4] = RGB5(0, 0, 0);
  OBJ_COLORS[SPARK_PALETTE_BASE + 5] = RGB5(20, 20, 20);
  OBJ_COLORS[SPARK_PALETTE_BASE + 6] = RGB5(28, 28, 28);
  OBJ_COLORS[SPARK_PALETTE_BASE + 7] = RGB5(31, 31, 31);

  // Set the attributes for spark sprite.
  g_shadow_attrs[SPARK_ATTR_BASE].attr0 = OBJ_Y(0) | ATTR0_DISABLED | ATTR0_TYPE_NORMAL | ATTR0_COLOR_16 | ATTR0_SQUARE;
  g_shadow_attrs[SPARK_ATTR_BASE].attr1 = OBJ_X(0) | ATTR1_SIZE_8;
  g_shadow_attrs[SPARK_ATTR_BASE].attr2 = OBJ_CHAR(SPARK_DATA_BASE) | ATTR2_PRIORITY(0) | ATTR2_PALETTE(SPARK_PALETTE_IDX);
}

// -------------------------------------------------------------------------------------------------

void init_puzzle_sprites() {
  init_cursor_sprite();
  init_spark_sprite();
}

// -------------------------------------------------------------------------------------------------
