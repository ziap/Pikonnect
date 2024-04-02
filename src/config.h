#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

const int LEVEL_COUNT = 6;

enum GameMode {
  GAMEMODE_CLASSIC,
  GAMEMODE_COLLAPSE,
  GAMEMODE_LEN
};

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 768;
const int HEADER_HEIGHT = 96;

const uint64_t MAGIC = 0x9e3779b97f4a7c55;

const float DAS = 0.17;
const float ARR = 0.07;

#endif
