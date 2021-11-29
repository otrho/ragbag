#include "prng.h"

static u16 g_prng = 2222;

void prng_seed(u16 seed) {
  g_prng = seed;
}

u16 prng() {
  g_prng = ((((g_prng >> 9) & 1) ^ ((g_prng >> 1) & 1)) << 15) | (g_prng >> 1);
  return g_prng;
}
