#include "scene_game.h"
#include "utils.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

struct LevelConfig {
  int height;
  int width;
  int num_classes;
};

static LevelConfig configs[LEVEL_COUNT] = {
  { 6,  6,  6 },
  { 6,  8,  9 },
  { 7,  10, 13 },
  { 8,  11, 16 },
  { 9,  12, 20 },
  { 10, 14, 26 },
};

void Scene_game_load(Game *game) {
  GameMenu *menu = &game->menu.game;
  menu->start_time = GetTime() * 1e6;

  int level = game->config.level;
  int gamemode = game->config.gamemode;

  LevelConfig config = configs[level];
  menu->board.width = config.width;
  menu->board.height = config.height;
  menu->board.data = (int **)malloc((config.height + 4) * sizeof(int*));

  for (int i = 0; i < config.height + 4; ++i) {
    menu->board.data[i] = (int*)malloc((config.width + 4) * sizeof(int));
  }

  for (int i = 0; i < config.height + 4; ++i) {
    menu->board.data[i][0] = -1;
    menu->board.data[i][config.width + 3] = -1;
  }

  for (int i = 1; i < config.width + 3; ++i) {
    menu->board.data[0][i] = -1;
    menu->board.data[config.height + 3][i] = -1;
  }

  for (int i = 1; i < config.height + 3; ++i) {
    menu->board.data[i][1] = 0;
    menu->board.data[i][config.width + 2] = 0;
  }

  for (int i = 2; i < config.width + 2; ++i) {
    menu->board.data[1][i] = 0;
    menu->board.data[config.height + 2][i] = 0;
  }

  int idx = 0;
  for (int i = 0; i < config.height; ++i) {
    for (int j = 0; j < config.width; ++j) {
      menu->board.data[i + 2][j + 2] = (idx++ / 2) % config.num_classes + 1;
    }
  }

  menu->random_state = game->current_user->info.random_state[level][gamemode];

  int total = config.height * config.width;
  for (int i = 0; i < total - 1; ++i) {
    uint32_t j = pcg32_bounded(&menu->random_state, total - i) + i;

    int x1 = i % config.width;
    int y1 = i / config.width;

    int x2 = j % config.width;
    int y2 = j / config.width;

    int t = menu->board.data[y1 + 2][x1 + 2];
    menu->board.data[y1 + 2][x1 + 2] = menu->board.data[y2 + 2][x2 + 2];
    menu->board.data[y2 + 2][x2 + 2] = t;
  }

  for (int i = 0; i < menu->board.height + 4; ++i) {
    for (int j = 0; j < menu->board.width + 4; ++j) {
      printf("%d ", menu->board.data[i][j]);
    }
    printf("\n");
  }
}

Scene Scene_game_update(Game *game, float dt) {
  (void)dt;
  GameMenu *menu = &game->menu.game;
  uint32_t time_ms = GetTime() * 1e6 - menu->start_time;
  uint32_t time_s = time_ms / 1000000;
  int s = time_s % 60;
  int m = time_s / 60;

  DrawRectangle(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, LIGHTGRAY);
  char msg[1024];
  snprintf(msg, sizeof(msg), "Level: %d", game->config.level + 1);
  DrawText(msg, 32, 32, 32, BLACK);

  snprintf(msg, sizeof(msg), "%02d:%02d", m, s);
  DrawText(msg, SCREEN_WIDTH - MeasureText(msg, 32) - 32, 32, 32, BLACK);

  return SCENE_GAME;
}

void Scene_game_unload(Game *game) {
  GameMenu *menu = &game->menu.game;

  for (int i = 0; i < menu->board.height + 4; ++i) {
    free(menu->board.data[i]);
  }
  free(menu->board.data);
  (void)game;
}

