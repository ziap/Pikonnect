#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define LEVEL_COUNT 6

typedef enum {
  GAMEMODE_CLASSIC,
  GAMEMODE_COLLAPSE,
  GAMEMODE_LEN
} GameMode;

#define HEADER_HEIGHT 96

#define MAGIC 0x9e3779b97f4a7c55

#define DAS 0.17f
#define ARR 0.07f

#define TUTORIAL_TRANSITION 0.3f
#define TUTORIAL_COOLDOWN   3.0f

#endif
