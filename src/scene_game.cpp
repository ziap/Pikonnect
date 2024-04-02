#include "scene_game.h"
#include "utils.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "search.h"

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

static const Color palette_dark[26] = {
  {45, 71, 61, 255},
  {41, 70, 65, 255},
  {37, 69, 69, 255},
  {34, 66, 70, 255},
  {32, 62, 71, 255},
  {32, 57, 70, 255},
  {32, 53, 68, 255},
  {34, 47, 65, 255},
  {36, 43, 60, 255},
  {39, 38, 55, 255},
  {44, 34, 50, 255},
  {48, 31, 44, 255},
  {53, 30, 38, 255},
  {57, 29, 33, 255},
  {62, 30, 29, 255},
  {66, 31, 26, 255},
  {68, 34, 24, 255},
  {70, 38, 23, 255},
  {71, 43, 24, 255},
  {71, 47, 26, 255},
  {69, 53, 30, 255},
  {67, 57, 34, 255},
  {63, 62, 39, 255},
  {59, 66, 45, 255},
  {55, 69, 51, 255},
  {50, 70, 56, 255},
};

void Scene_game_load(Game *game) {
  GameMenu *menu = &game->menu.game;
  menu->start_time = GetTime() * 1e6;
  menu->position.x = 0;
  menu->position.y = 0;
  menu->selecting = false;
  menu->path_lerp = 0;
  menu->path.len = 0;

  int level = game->config.level;

  LevelConfig config = configs[level];
  GameBoard_init(&menu->board, config.width, config.height);

  menu->board_lerp = (float*)calloc(config.height * config.width, sizeof(float));

  uint64_t *random_state = &game->current_user->info.random_state;
  int idx = pcg32_bounded(random_state, config.num_classes) * 2;
  for (int i = 0; i < config.height; ++i) {
    for (int j = 0; j < config.width; ++j) {
      *GameBoard_index(menu->board, {i, j}) = (idx++ / 2) % config.num_classes + 1;
    }
  }

  int total = config.height * config.width;
  for (int i = 0; i < total - 1; ++i) {
    int j = pcg32_bounded(random_state, total - i) + i;
    int *p1 = GameBoard_index(menu->board, {i / config.width, i % config.width});
    int *p2 = GameBoard_index(menu->board, {j / config.width, j % config.width});

    int t = *p1;
    *p1 = *p2;
    *p2 = t;
  }
}

static const Dir opposite[DIR_LEN] = {DIR_DOWN, DIR_RIGHT, DIR_UP, DIR_LEFT};

Scene Scene_game_update(Game *game, float dt) {
  (void)dt;
  GameMenu *menu = &game->menu.game;
  const uint32_t time_ms = GetTime() * 1e6 - menu->start_time;
  const uint32_t time_s = time_ms / 1000000;
  const int s = time_s % 60;
  const int m = time_s / 60;

  const KeyboardKey keys[DIR_LEN][3] = {
    {KEY_UP, KEY_K, KEY_W},
    {KEY_LEFT, KEY_H, KEY_A},
    {KEY_DOWN, KEY_J, KEY_S},
    {KEY_RIGHT, KEY_L, KEY_D},
  };

  bool dispatching[DIR_LEN] = {0};
  bool pressing[DIR_LEN] = {0};

  for (int dir = 0; dir < DIR_LEN; ++dir) {
    for (int i = 0; i < 3; ++i) {
      if (IsKeyDown(keys[dir][i])) pressing[dir] = true;
    }
  }

  for (int dir = 0; dir < DIR_LEN; ++dir) {
    if (pressing[dir]) {
      if (!menu->dispatched[dir]) {
        dispatching[dir] = true;
        menu->dispatched[dir] = true;
      }

    } else {
      menu->dispatched[dir] = false;
    }
  }

  Index next_position = menu->position;

  for (int dir = 0; dir < DIR_LEN; ++dir) {
    if (dispatching[dir]) {
      menu->as_delay[dir] = DAS;
      menu->move_direction = (Dir)dir;

      next_position = Index_step(next_position, (Dir)dir);
    } else if (pressing[dir]) {
      if (!pressing[opposite[dir]]) {
        menu->move_direction = (Dir)dir;
      }

      if (menu->move_direction == dir) {
        menu->as_delay[dir] -= dt;
        if (menu->as_delay[dir] <= 0) {
          menu->as_delay[dir] = ARR;
          next_position = Index_step(next_position, (Dir)dir);
        }
      }
    }
  }

  if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
    if (*GameBoard_index(menu->board, menu->position)) {
      if (!menu->selecting) {
        menu->selecting = true;
        menu->selection = menu->position;
      } else {
        Path path = Search_valid_path(menu->board, menu->position, menu->selection);

        if (path.len > 0) {
          menu->selecting = false;
          menu->path = path;
          menu->path_val = *GameBoard_index(menu->board, menu->position);
          menu->path_lerp = 0;

          *GameBoard_index(menu->board, menu->position) = 0;
          *GameBoard_index(menu->board, menu->selection) = 0;
        } else {
          menu->selection = menu->position;
        }
      }
    }
  }

  if (IsKeyPressed(KEY_X)) {
    menu->start_time -= 60e6;
    Path path = Search_suggest_move(menu->board);
    if (path.len >= 2) {
      menu->selecting = false;
      menu->path = path;
      menu->path_val = *GameBoard_index(menu->board, path.data[0]);
      menu->path_lerp = 0;

      *GameBoard_index(menu->board, path.data[0]) = 0;
      *GameBoard_index(menu->board, path.data[path.len - 1]) = 0;
    }
  }

  const int w = menu->board.width;
  const int h = menu->board.height;

  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      float *lerp = menu->board_lerp + (i * w + j);

      if (menu->selecting && i == menu->selection.y && j == menu->selection.x) {
        *lerp += 5 * dt;
        if (*lerp > 1) *lerp = 1;
      } else {
        *lerp -= 5 * dt;
        if (*lerp < 0) *lerp = 0;
      }
    }

  }

  menu->path_lerp += 3 * dt;
  if (menu->path_lerp > 1) {
    menu->path.len = 0;
  }

  menu->position = next_position;
  if (menu->position.x < 0) menu->position.x = 0;
  if (menu->position.y < 0) menu->position.y = 0;

  if (menu->position.x >= w) menu->position.x = w - 1;
  if (menu->position.y >= h) menu->position.y = h - 1;

  const int gap_x = 8 * (w - 1);
  const int gap_y = 8 * (h - 1);

  const int side_x = (SCREEN_WIDTH - gap_x) / (w + 2);
  const int side_y = (SCREEN_HEIGHT - HEADER_HEIGHT - gap_y) / (h + 2);

  const int GRID_SIDE = side_x < side_y ? side_x : side_y;

  const int x0 = (SCREEN_WIDTH - GRID_SIDE * w - gap_x) / 2;
  const int y0 = HEADER_HEIGHT + (SCREEN_HEIGHT - HEADER_HEIGHT - GRID_SIDE * h - gap_y) / 2;

  DrawRectangle(x0 + (GRID_SIDE + 8) * menu->position.x - 5,
                y0 + (GRID_SIDE + 8) * menu->position.y - 5, GRID_SIDE + 10,
                GRID_SIDE + 10, GRAY);

  int y = y0;
  for (int i = 0; i < h; ++i) {
    int x = x0;
    for (int j = 0; j < w; ++j) {
      int val = *GameBoard_index(menu->board, {i, j});

      DrawRectangle(x, y, GRID_SIDE, GRID_SIDE, WHITE);
      if (val) {
        float t = easing_cubic(menu->board_lerp[i * w + j]);
        
        const float px = x + GRID_SIDE * t * 0.1f;
        const float py = y + GRID_SIDE * t * 0.1f;
        const float side = GRID_SIDE * (1 - t * 0.2f);
        const int GRID_TEXT = side * 6 / 10;
        const int GRID_PAD  = side * 2 / 10;
        DrawRectangle(px, py, side, side, palette[val - 1]);
        char c[2] = { (char)('A' + val - 1) , '\0' };
        DrawText(c, px + 0.5f * (side - MeasureText(c, GRID_TEXT)), py + GRID_PAD, GRID_TEXT, palette_dark[val - 1]);
      }

      x += GRID_SIDE + 8;
    }

    y += GRID_SIDE + 8;
  }

  if (menu->path.len) {
    float x = 2 * menu->path_lerp - 1;
    x *= x;

    Color path_color = palette[menu->path_val - 1];
    path_color.g += (255 - path_color.g) * x;
    path_color.r += (255 - path_color.r) * x;
    path_color.b += (255 - path_color.b) * x;

    for (int i = 0; i < menu->path.len; ++i) {
      Index idx = menu->path.data[i];
      DrawCircle(x0 + (GRID_SIDE + 8) * idx.x + GRID_SIDE / 2,
                 y0 + (GRID_SIDE + 8) * idx.y + GRID_SIDE / 2,
                 GRID_SIDE * 0.25f, path_color);
    }

    for (int i = 1; i < menu->path.len; ++i) {
      Index c0 = menu->path.data[i - 1];
      Index c1 = menu->path.data[i];

      Vector2 p0 = {
        x0 + (GRID_SIDE + 8) * c0.x + GRID_SIDE * 0.5f,
        y0 + (GRID_SIDE + 8) * c0.y + GRID_SIDE * 0.5f,
      };

      Vector2 p1 = {
        x0 + (GRID_SIDE + 8) * c1.x + GRID_SIDE * 0.5f,
        y0 + (GRID_SIDE + 8) * c1.y + GRID_SIDE * 0.5f,
      };
      DrawLineEx(p0, p1, GRID_SIDE * 0.2, path_color);
    }
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
  free(menu->board_lerp);
  GameBoard_deinit(&menu->board);
  (void)game;
}

