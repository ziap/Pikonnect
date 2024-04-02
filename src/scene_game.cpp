#include "scene_game.h"
#include "utils.h"
#include "palette.h"
#include <raylib.h>
#include <stdio.h>

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

// Compute the score bonus based on the time to find a match
//
// f(t) = c0 * t^5 + c1 * t^4 + c2 * t^3 + c3 * t^2 + c4 * t + c5
// f(0) = 10
// f(10) = 1
// f'(0) = f'(10) = f''(0) = f''(10) = 0
static int score(float t, int s) {
  if (t > 10) return s;
  return (int)(525000 - ((27 * t - 675) * t + 4500) * t * t * t) * s / 50000;
}

void Scene_game_load(Game *game) {
  GameMenu *menu = &game->menu.game;
  menu->pos.x = 0;
  menu->pos.y = 0;

  menu->selecting = false;
  menu->selection_lerp = 0;

  menu->path_lerp = 0;
  menu->path.len = 0;
  menu->score = 0;
  menu->score_timer = 0;

  int level = game->config.level;

  LevelConfig config = configs[level];
  GameBoard_init(&menu->board, config.width, config.height);

  menu->remaining = config.width * config.height;

  uint64_t *random_state = &game->current_user->info.random_state;
  int idx = pcg32_bounded(random_state, config.num_classes) * 2;
  for (int i = 0; i < config.height; ++i) {
    for (int j = 0; j < config.width; ++j) {
      int *tile = GameBoard_index(menu->board, {i, j});
      *tile = (idx++ / 2) % config.num_classes + 1;
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

Scene Scene_game_update(Game *game, float dt) {
  (void)dt;
  GameMenu *menu = &game->menu.game;

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

  Index next_position = menu->pos;

  const Dir opposite[DIR_LEN] = {DIR_DOWN, DIR_RIGHT, DIR_UP, DIR_LEFT};
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
    if (*GameBoard_index(menu->board, menu->pos)) {
      if (!menu->selecting) {
        menu->selecting = true;
        menu->selection = menu->pos;
        menu->selection_lerp = 1;
      } else {
        Path path = Search_valid_path(menu->board, menu->pos, menu->selection);

        if (path.len > 0) {
          menu->selecting = false;
          menu->path = path;
          menu->path_val = *GameBoard_index(menu->board, menu->pos);
          menu->path_lerp = 0;
        
          int len = 1;
          for (int i = 1; i < path.len; ++i) {
            Index p0 = path.data[i - 1];
            Index p1 = path.data[i];

            len += p1.x > p0.x ? p1.x - p0.x : p0.x - p1.x;
            len += p1.y > p0.y ? p1.y - p0.y : p0.y - p1.y;
          }

          menu->score += score(menu->score_timer, len);
          menu->score_timer = 0;

          *GameBoard_index(menu->board, menu->pos) = 0;
          *GameBoard_index(menu->board, menu->selection) = 0;
          menu->remaining -= 2;

          if (menu->remaining ==0 || Search_suggest_move(menu->board).len == 0) {
            printf("Game over\n");
          }
        } else {
          menu->selection = menu->pos;
          menu->selection_lerp = 1;
        }
      }
    }
  }

  menu->score_timer += dt;

  if (IsKeyPressed(KEY_X)) {
    Path path = Search_suggest_move(menu->board);
    if (path.len >= 2) {
      menu->selecting = false;
      menu->path = path;
      menu->path_val = *GameBoard_index(menu->board, path.data[0]);
      menu->path_lerp = 0;

      *GameBoard_index(menu->board, path.data[0]) = 0;
      *GameBoard_index(menu->board, path.data[path.len - 1]) = 0;
      menu->remaining -= 2;

      if (menu->remaining ==0 || Search_suggest_move(menu->board).len == 0) {
        printf("Game over\n");
      }
    }
  }

  const int w = menu->board.width;
  const int h = menu->board.height;

  menu->path_lerp += 3 * dt;
  if (menu->path_lerp > 1) {
    menu->path.len = 0;
  }

  menu->selection_lerp -= 5 * dt;
  if (menu->selection_lerp < 0) menu->selection_lerp = 0;

  menu->pos = next_position;
  if (menu->pos.x < 0) menu->pos.x = 0;
  if (menu->pos.y < 0) menu->pos.y = 0;

  if (menu->pos.x >= w) menu->pos.x = w - 1;
  if (menu->pos.y >= h) menu->pos.y = h - 1;

  const int gap_x = 8 * (w - 1);
  const int gap_y = 8 * (h - 1);

  const int side_x = (SCREEN_WIDTH - gap_x) / (w + 2);
  const int side_y = (SCREEN_HEIGHT - HEADER_HEIGHT - gap_y) / (h + 2);

  const int grid_side = side_x < side_y ? side_x : side_y;

  const int x0 = (SCREEN_WIDTH - grid_side * w - gap_x) / 2;
  const int y0 = (SCREEN_HEIGHT + HEADER_HEIGHT - grid_side * h - gap_y) / 2;

  DrawRectangle(x0 + (grid_side + 8) * menu->pos.x - 5,
                y0 + (grid_side + 8) * menu->pos.y - 5, grid_side + 10,
                grid_side + 10, GRAY);

  const int GRID_TEXT = grid_side * 0.6;
  const int GRID_PAD = grid_side * 0.2;
  int y = y0;
  for (int i = 0; i < h; ++i) {
    int x = x0;
    for (int j = 0; j < w; ++j) {
      int val = *GameBoard_index(menu->board, {i, j});

      DrawRectangle(x, y, grid_side, grid_side, WHITE);
      if (val) {
        DrawRectangle(x, y, grid_side, grid_side, palette[val - 1]);
        char c[2] = { (char)('A' + val - 1) , '\0' };
        DrawText(c, x + 0.5f * (grid_side - MeasureText(c, GRID_TEXT)),
                 y + GRID_PAD, GRID_TEXT, palette_dark[val - 1]);
      }

      x += grid_side + 8;
    }

    y += grid_side + 8;
  }

  if (menu->selecting) {
    int cursor_width = grid_side / 3;

    float t = smoothstep(menu->selection_lerp);

    int sx = x0 + (grid_side + 8) * menu->selection.x;
    int sy = y0 + (grid_side + 8) * menu->selection.y;

    Color c = palette[*GameBoard_index(menu->board, menu->selection) - 1];
    c.r = c.r * 2 / 3;
    c.g = c.g * 2 / 3;
    c.b = c.b * 2 / 3;
    
    int x0 = sx - 4 - 4 * t;
    int x1 = sx + 4 - 4 * t;

    int x2 = sx + grid_side - cursor_width - 4 + 4 * t;
    int x3 = sx + grid_side - 4 + 4 * t;

    int y0 = sy - 4 - 4 * t;
    int y2 = sy + grid_side - cursor_width - 4 + 4 * t;
    int y1 = sy + 4 - 4 * t;
    int y3 = sy + grid_side - 4 + 4 * t;

    DrawRectangle(x0, y0, 8, 8 + cursor_width, c);
    DrawRectangle(x1, y0, cursor_width, 8, c);

    DrawRectangle(x2, y0, 8 + cursor_width, 8, c);
    DrawRectangle(x3, y1, 8, cursor_width, c);

    DrawRectangle(x0, y2, 8, 8 + cursor_width, c);
    DrawRectangle(x1, y3, cursor_width, 8, c);

    DrawRectangle(x2, y3, 8 + cursor_width, 8, c);
    DrawRectangle(x3, y2, 8, cursor_width, c);
  }

  const float center = grid_side * 0.5f;

  if (menu->path.len) {
    float x = 2 * menu->path_lerp - 1;
    x *= x;

    Color path_color = palette[menu->path_val - 1];
    path_color.g += (255 - path_color.g) * x;
    path_color.r += (255 - path_color.r) * x;
    path_color.b += (255 - path_color.b) * x;

    for (int i = 0; i < menu->path.len; ++i) {
      Index idx = menu->path.data[i];
      DrawCircle(x0 + (grid_side + 8) * idx.x + center,
                 y0 + (grid_side + 8) * idx.y + center,
                 grid_side * 0.125f, path_color);
    }

    for (int i = 1; i < menu->path.len; ++i) {
      Index c0 = menu->path.data[i - 1];
      Index c1 = menu->path.data[i];

      Vector2 p0 = {
        x0 + (grid_side + 8) * c0.x + center,
        y0 + (grid_side + 8) * c0.y + center,
      };

      Vector2 p1 = {
        x0 + (grid_side + 8) * c1.x + center,
        y0 + (grid_side + 8) * c1.y + center,
      };
      DrawLineEx(p0, p1, grid_side * 0.12, path_color);
    }
  }

  DrawRectangle(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, LIGHTGRAY);
  char msg[1024];
  snprintf(msg, sizeof(msg), "Level: %d", game->config.level + 1);
  DrawText(msg, 32, 32, 32, BLACK);

  snprintf(msg, sizeof(msg), "Score: %d", menu->score);
  DrawText(msg, SCREEN_WIDTH - MeasureText(msg, 32) - 32, 32, 32, BLACK);

  return SCENE_GAME;
}

void Scene_game_unload(Game *game) {
  GameMenu *menu = &game->menu.game;
  GameBoard_deinit(&menu->board);
  (void)game;
}

