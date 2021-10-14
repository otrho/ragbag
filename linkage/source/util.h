#pragma once

// Wacky XOR method for min/max.  Better than a ternary op?
#define min(a, b) ((b) ^ (((a) ^ (b)) & -((a) < (b))))
#define max(a, b) ((a) ^ (((a) ^ (b)) & -((a) < (b))))
