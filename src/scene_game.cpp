#include "scene_game.h"
#include "utils.h"
#include <raylib.h>
#include <stdio.h>

struct LevelConfig {
  int height;
  int width;
  int num_classes;
};

static const LevelConfig configs[LEVEL_COUNT] = {
  {  6,  6,  6 },
  {  6,  8,  9 },
  {  7, 10, 13 },
  {  8, 11, 16 },
  {  9, 12, 20 },
  { 10, 14, 26 },
};

// r = 185 + 70 * sin(2pi * (0.55 + x / 26))
// g = 180 + 75 * sin(2pi * (0.25 + x / 26))
// b = 170 + 85 * sin(2pi * (0.10 + x / 26))
//
// Technique from: https://iquilezles.org/articles/palettes/
static const Color palette[26] = {
  { 163, 255, 219, 255 },
  { 148, 252, 234, 255 },
  { 134, 246, 246, 255 },
  { 124, 236, 252, 255 },
  { 117, 222, 254, 255 },
  { 115, 206, 252, 255 },
  { 116, 189, 244, 255 },
  { 121, 170, 232, 255 },
  { 130, 153, 216, 255 },
  { 142, 137, 198, 255 },
  { 157, 123, 178, 255 },
  { 173, 113, 157, 255 },
  { 190, 107, 137, 255 },
  { 206, 105, 120, 255 },
  { 221, 107, 105, 255 },
  { 235, 113,  93, 255 },
  { 245, 123,  87, 255 },
  { 252, 137,  85, 255 },
  { 254, 153,  87, 255 },
  { 253, 170,  95, 255 },
  { 248, 189, 107, 255 },
  { 239, 206, 123, 255 },
  { 227, 222, 141, 255 },
  { 212, 236, 161, 255 },
  { 196, 246, 182, 255 },
  { 179, 252, 202, 255 },
};

void Scene_game_load(Game *game) {
  GameMenu *menu = &game->menu.game;
  menu->start_time = GetTime() * 1e6;

  int level = game->config.level;
  int gamemode = game->config.gamemode;

  LevelConfig config = configs[level];
  GameBoard_init(&menu->board, config.width, config.height);

  int idx = 0;
  for (int i = 0; i < config.height; ++i) {
    for (int j = 0; j < config.width; ++j) {
      *GameBoard_index(&menu->board, {i, j}) = (idx++ / 2) % config.num_classes + 1;
    }
  }

  uint64_t random_state = game->current_user->info.random_state[level][gamemode];

  int total = config.height * config.width;
  for (int i = 0; i < total - 1; ++i) {
    int j = pcg32_bounded(&random_state, total - i) + i;
    int *p1 = GameBoard_index(&menu->board, {i / config.width, i % config.width});
    int *p2 = GameBoard_index(&menu->board, {j / config.width, j % config.width});

    int t = *p1;
    *p1 = *p2;
    *p2 = t;
  }

  menu->random_state = random_state;
}

Scene Scene_game_update(Game *game, float dt) {
  (void)dt;
  GameMenu *menu = &game->menu.game;
  const uint32_t time_ms = GetTime() * 1e6 - menu->start_time;
  const uint32_t time_s = time_ms / 1000000;
  const int s = time_s % 60;
  const int m = time_s / 60;

  const int gap_x = 8 * (menu->board.width - 1);
  const int gap_y = 8 * (menu->board.height - 1);

  const int side_x = (SCREEN_WIDTH - 64 - gap_x) / menu->board.width;
  const int side_y = (SCREEN_HEIGHT - HEADER_HEIGHT - 64 - gap_y) / menu->board.height;

  const int GRID_SIDE = side_x < side_y ? side_x : side_y;
  const int GRID_TEXT = GRID_SIDE * 6 / 10;
  const int GRID_PAD  = GRID_SIDE * 2 / 10;

  const int x0 = (SCREEN_WIDTH - GRID_SIDE * menu->board.width - gap_x) / 2;

  int y = HEADER_HEIGHT + (SCREEN_HEIGHT - HEADER_HEIGHT - GRID_SIDE * menu->board.height - gap_y) / 2;
  for (int i = 0; i < menu->board.height; ++i) {
    int x = x0;
    for (int j = 0; j < menu->board.width; ++j) {
      int val = *GameBoard_index(&menu->board, {i, j});
      if (val) {
        DrawRectangle(x, y, GRID_SIDE, GRID_SIDE, palette[val - 1]);
        char c[2] = { (char)('A' + val - 1) , '\0' };
        DrawText(c, x + (GRID_SIDE - MeasureText(c, GRID_TEXT)) / 2, y + GRID_PAD, GRID_TEXT, BLACK);
      }

      x += GRID_SIDE + 8;
    }

    y += GRID_SIDE + 8;
  }

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

  GameBoard_deinit(&menu->board);
  (void)game;
}

