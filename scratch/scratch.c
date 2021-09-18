#include <gba.h>

static void fill_screen(u16 colour) {
#define VID_BUF ((u16*)VRAM)
  // memset() or Tonc's memset16() would be better?
  for (int pix_idx = 0; pix_idx < SCREEN_WIDTH * SCREEN_HEIGHT; ++pix_idx) {
    VID_BUF[pix_idx] = colour;
  }
}

int main() {
  irqInit();
  irqEnable(IRQ_VBLANK);
  REG_IME = 1;

  SetMode(MODE_3 | BG2_ENABLE);

  u16 colour = 0;
  while(1) {
    VBlankIntrWait();

    scanKeys();
    u16 keys_down = keysDown();
    u16 keys_up = keysUp();

    if (keys_down & KEY_A) {
      colour |= 0x001f;
    }
    if (keys_up & KEY_A) {
      colour &= ~0x001f;
    }
    if (keys_down & KEY_B) {
      colour |= (0x1f << 10);
    }
    if (keys_up & KEY_B) {
      colour &= ~(0x001f << 10);
    }

    fill_screen(colour);

    if (keys_down & KEY_START) {
      break;
    }
  }

  Stop();
  return 0;
}
