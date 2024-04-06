#ifndef PALETTE_H
#define PALETTE_H

#include <raylib.h>

// r = 185 + 70 * sin(2pi * (0.55 + x / 26))
// r = 185 + 70 * sin(2pi * (0.55 + x / 26))
// g = 180 + 75 * sin(2pi * (0.25 + x / 26))
// b = 170 + 85 * sin(2pi * (0.10 + x / 26))
//
// Technique from: https://iquilezles.org/articles/palettes/
extern const Color palette[];
extern const Color palette_dark[];
extern const Color palette_select[];

#endif
