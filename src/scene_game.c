#include "scene_game.h"

#include <math.h>
#include <stdio.h>

#include "utils.h"
#include "palette.h"
#include "search.h"

typedef struct {
  int height;
  int width;
  int num_classes;
} LevelConfig;

static const LevelConfig configs[LEVEL_COUNT] = {
  { .height =  6, .width =  6, .num_classes =  6 },
  { .height =  6, .width =  8, .num_classes =  9 },
  { .height =  7, .width = 10, .num_classes = 13 },
  { .height =  8, .width = 11, .num_classes = 16 },
  { .height =  9, .width = 12, .num_classes = 20 },
  { .height = 10, .width = 14, .num_classes = 26 },
};

typedef enum {
  STATUS_PLAYING,
  STATUS_WON,
  STATUS_LOST
} GameStatus;

// This is repeated quite often so we put it in a function
static Index tile_position(GameMenu *menu, Index tile) {
  return (Index) {
    .y = menu->y0 + (menu->grid_side + 8) * tile.y,
    .x = menu->x0 + (menu->grid_side + 8) * tile.x,
  };
}

// Update the size every frame to take into account of:
// - Row/column deletion
// - Window resizing
static void update_size(GameMenu *menu, float w, float h) {
  float t = menu->collapse_timer - 1;
  t *= t;

  if (menu->collapse_timer < 2) {
    w += menu->collapse_col_len;
    h += menu->collapse_row_len;

    if (menu->collapse_timer > 1) {
      w -= t * menu->collapse_col_len;
      h -= t * menu->collapse_row_len;
    }
  }

  const float gap_x = 8 * (w - 1);
  const float gap_y = 8 * (h - 1);

  const float side_x = (GetScreenWidth() - gap_x) / (w + 2);
  const float side_y = (GetScreenHeight() - HEADER_HEIGHT - gap_y) / (h + 2);

  menu->grid_side = side_x < side_y ? side_x : side_y;
  if (menu->grid_side > 96) menu->grid_side = 96;

  menu->x0 = (GetScreenWidth() - menu->grid_side * w - gap_x) / 2;
  menu->y0 = (GetScreenHeight() + HEADER_HEIGHT - menu->grid_side * h - gap_y) / 2;
}

static GameStatus remove_pair(Game *game, Path path) {
  Index p1 = path.data[0];
  Index p2 = path.data[path.len - 1];
  GameMenu *menu = &game->menu.game;

  menu->board.data[p1.y][p1.x] = 0;
  menu->board.data[p2.y][p2.x] = 0;
  menu->remaining -= 2;

  if (game->config.gamemode == GAMEMODE_COLLAPSE) {
    // The newly-created empty row/column contains the previously deleted tiles
    // So the rows and columns of those tiles are checked and might be removed
    // However, if two tiles share the same row/column then only the first
    // tile's row/column are checked
    bool remove_second_x = p2.x != p1.x;
    bool remove_second_y = p2.y != p1.y;
    menu->collapse_row_len = 0;
    menu->collapse_col_len = 0;
    menu->collapse_timer = 0;

    // Shift everything backward, including the collapse position
    if (GameBoard_remove_row(&menu->board, p1.y)) {
      if (p2.y > p1.y) --p2.y;
      if (menu->pos.y > p1.y) --menu->pos.y;
      menu->collapse_row[menu->collapse_row_len++] = p1.y;
    }
    if (GameBoard_remove_col(&menu->board, p1.x)) {
      if (p2.x > p1.x) --p2.x;
      if (menu->pos.x > p1.x) --menu->pos.x;
      menu->collapse_col[menu->collapse_col_len++] = p1.x;
    }
    if (remove_second_y && GameBoard_remove_row(&menu->board, p2.y)) {
      if (menu->pos.y > p2.y) --menu->pos.y;
      if (menu->collapse_row_len && menu->collapse_row[0] > p2.y) --menu->collapse_row[0];
      menu->collapse_row[menu->collapse_row_len++] = p2.y;
    }
    if (remove_second_x && GameBoard_remove_col(&menu->board, p2.x)) {
      if (menu->pos.x > p2.x) --menu->pos.x;
      if (menu->collapse_col_len && menu->collapse_col[0] > p2.x) --menu->collapse_col[0];
      menu->collapse_col[menu->collapse_col_len++] = p2.x;
    }
  }

  menu->path = path;
  menu->path_lerp = 0;

  // Only check for game over after a move
  if (menu->remaining == 0) {
    UserInfo *info = &game->current_user->info;
    if (info->unlocked[game->config.gamemode] <= game->config.level) {
      info->unlocked[game->config.gamemode] = game->config.level + 1;
    }

    game->result.new_best = info->best_score < menu->score;
    if (game->result.new_best) {
      info->best_score = menu->score;
    }

    game->result.last_score = menu->score;
    return STATUS_WON;
  }

  if (Search_suggest_move(menu->board, &menu->search_queue).len == 0) {
    return STATUS_LOST;
  }

  return STATUS_PLAYING;
}

// Compute the score bonus based on the time to find a match
//
// f(t, d) = (c0 * t^5 + c1 * t^4 + c2 * t^3 + c3 * t^2 + c4 * t + c5) * d
//
// f(0, d) = 10 * d
// f(5, d) = 5 * d
// f(10, d) = d
// f'(0, d) = f'(10, d) = f''(10, d) = 0
static int score(float t, int s) {
  if (t > 10) return s;
  return (int)((500000 - ((27 * t - 675) * t + 4500) * t * t * t) * s + 25000) / 50000;
}

// Delayed auto shift game controller
static void update_move(Game *game, float dt) {
  GameMenu *menu = &game->menu.game;
  const KeyboardKey keys[DIR_LEN][3] = {
    [DIR_UP] = {KEY_UP, KEY_K, KEY_W},
    [DIR_LEFT] = {KEY_LEFT, KEY_H, KEY_A},
    [DIR_DOWN] = {KEY_DOWN, KEY_J, KEY_S},
    [DIR_RIGHT] = {KEY_RIGHT, KEY_L, KEY_D},
  };

  // Check if directional keys are being pressed
  bool pressing[DIR_LEN] = {0};
  for (int dir = 0; dir < DIR_LEN; ++dir) {
    for (int i = 0; i < 3; ++i) {
      if (IsKeyDown(keys[dir][i])) pressing[dir] = true;
    }
  }

  // Check if directional keys are being pressed this frame
  bool dispatching[DIR_LEN] = {0};
  for (Dir dir = 0; dir < DIR_LEN; ++dir) {
    if (pressing[dir]) {
      if (!menu->dispatched[dir]) {
        dispatching[dir] = true;
        menu->dispatched[dir] = true;
      }

    } else {
      menu->dispatched[dir] = false;
    }
  }

  const Dir opposite[DIR_LEN] = {
    [DIR_UP] = DIR_DOWN,
    [DIR_LEFT] = DIR_RIGHT,
    [DIR_DOWN] = DIR_UP,
    [DIR_RIGHT] = DIR_LEFT
  };
  for (Dir dir = 0; dir < DIR_LEN; ++dir) {
    if (dispatching[dir]) {
      menu->as_delay[dir] = DAS;
      menu->move_direction = (Dir)dir;

      PlaySound(game->sounds[SOUND_CLICK]);
      menu->pos = Index_step(menu->pos, (Dir)dir);
    } else if (pressing[dir]) {
      // Cancel the auto shift of the opposite direction
      if (!pressing[opposite[dir]]) {
        menu->move_direction = (Dir)dir;
      }

      if (menu->move_direction == dir) {
        menu->as_delay[dir] -= dt;
        if (menu->as_delay[dir] <= 0) {
          menu->as_delay[dir] = ARR;
          PlaySound(game->sounds[SOUND_CLICK]);
          menu->pos = Index_step(menu->pos, (Dir)dir);
        }
      }
    }
  }

  // Clamp the position
  if (menu->pos.x < 0) menu->pos.x = 0;
  if (menu->pos.y < 0) menu->pos.y = 0;

  if (menu->pos.x >= menu->board.width) menu->pos.x = menu->board.width - 1;
  if (menu->pos.y >= menu->board.height) menu->pos.y = menu->board.height - 1;
}

static GameStatus update_select(Game *game) {
  GameMenu *menu = &game->menu.game;

  if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
    if (menu->board.data[menu->pos.y][menu->pos.x]) {
      if (!menu->selecting) {
        // Select the first tile
        PlaySound(game->sounds[SOUND_SELECT]);
        menu->selecting = true;
        menu->selection = menu->pos;
        menu->selection_lerp = 1;
      } else {
        // Select the second tile
        Path path = Search_check_path(menu->board, menu->pos, menu->selection, &menu->search_queue);

        if (path.len > 0) {
          PlaySound(game->sounds[SOUND_CORRECT]);
          menu->selecting = false;
          menu->path_val = menu->board.data[menu->pos.y][menu->pos.x];

          // Compute the length of the path for scoring
          int len = 1;
          for (int i = 1; i < path.len; ++i) {
            Index p0 = path.data[i - 1];
            Index p1 = path.data[i];

            len += p1.x > p0.x ? p1.x - p0.x : p0.x - p1.x;
            len += p1.y > p0.y ? p1.y - p0.y : p0.y - p1.y;
          }

          if (!menu->hinting) {
            menu->score += score(menu->score_timer, len);
            menu->score_timer = 0;
          } else {
            menu->hinting = false;
          }

          return remove_pair(game, path);
        } else {
          // TODO: Use a different sound here
          PlaySound(game->sounds[SOUND_SELECT]);
          menu->selection = menu->pos;
          menu->selection_lerp = 1;
        }
      }
    }
  }

  return STATUS_PLAYING;
}

static void update_suggest(Game *game) {
  GameMenu *menu = &game->menu.game;

  if (menu->hinting) return;

  if (IsKeyPressed(KEY_E) || (menu->is_tutorial && menu->tutorial.stage == STAGE_CONNECT && menu->tutorial.timer >= TUTORIAL_COOLDOWN)) {
    PlaySound(game->sounds[SOUND_SELECT]);
    Path path = Search_suggest_move(menu->board, &menu->search_queue);
    if (path.len >= 2) {
      menu->hinting = true;
      menu->hint_timer = 0;
      menu->hint_indices[0] = path.data[0];
      menu->hint_indices[1] = path.data[path.len - 1];
    }
  }
}

// Update the timers independently for them to have different speeds and offsets
static void update_interpolation(GameMenu *menu, float dt) {
  menu->path_lerp += 5 * dt;
  if (menu->path_lerp > 1) {
    menu->path.len = 0;
  }

  menu->selection_lerp -= 5 * dt;
  if (menu->selection_lerp < 0) menu->selection_lerp = 0;

  menu->score_timer += dt;
  menu->hint_timer += dt;
  menu->collapse_timer += 5 * dt;
  menu->background_timer += 0.02f * dt;

  menu->tutorial.timer += dt;
}

static void render_background(GameMenu *menu) {
  int w = GetScreenWidth();
  int h = GetScreenHeight() - HEADER_HEIGHT;

  int side_min = (w > h ? w : h) / 36;
  int side_range = side_min * 4;
  int side_max = side_min + side_range;

  float t = menu->background_timer;

  for (int i = 0; i < BG_SQUARE_COUNT; ++i) {
    BackgroundSquare square = menu->background_squares[i];
    float side = side_min + side_range * square.side;
    float tx = t * square.freq_x;
    float ty = t * square.freq_y;

    int x = square.x * w - side * 0.5f + side_max * 4 * fbm(tx, square.fbm_offset_x);
    int y = HEADER_HEIGHT + square.y * h - side * 0.5f + side_max * 4 * fbm(ty, square.fbm_offset_y);
    Color c = palette[square.col];
    c.a = 192;
    DrawRectangle(x, y, side, side, c);
  }

  DrawRectangle(0, HEADER_HEIGHT, w, h, (Color) {255, 255, 255, 128});
}

static void render_position(GameMenu *menu) {
  Index pos = tile_position(menu, menu->pos);
  float t = menu->collapse_timer - 1;
  t *= t;

  if (menu->collapse_timer < 2) {
    for (int i = 0; i < menu->collapse_row_len; ++i) {
      if (menu->pos.y >= menu->collapse_row[i]) {
        pos.y += menu->grid_side + 8;
        if (menu->collapse_timer > 1) {
          pos.y -= t * (menu->grid_side + 8);
        }
      }
    }

    for (int i = 0; i < menu->collapse_col_len; ++i) {
      if (menu->pos.x >= menu->collapse_col[i]) {
        pos.x += menu->grid_side + 8;
        if (menu->collapse_timer > 1) {
          pos.x -= t * (menu->grid_side + 8);
        }
      }
    }
  }

  DrawRectangle(pos.x - 5, pos.y - 5, menu->grid_side + 5, 5, GRAY);
  DrawRectangle(pos.x + menu->grid_side, pos.y - 5, 5, menu->grid_side + 5, GRAY);
  DrawRectangle(pos.x, pos.y + menu->grid_side, menu->grid_side + 5, 5, GRAY);
  DrawRectangle(pos.x - 5, pos.y, 5, menu->grid_side + 5, GRAY);
}

static void render_board(GameMenu *menu) {
  const int grid_text = menu->grid_side * 0.6;
  const int grid_pad = menu->grid_side * 0.2;
  float t = menu->collapse_timer - 1;
  t *= t;
  int y = menu->y0;
  for (int i = 0; i < menu->board.height; ++i) {
    int x = menu->x0;
    // Add spaces between the previously deleted rows
    if (menu->collapse_timer < 2) {
      for (int j = 0; j < menu->collapse_row_len; ++j)  {
        if (i == menu->collapse_row[j]) {
          y += menu->grid_side + 8;
          if (menu->collapse_timer > 1) y -= t * (menu->grid_side + 8);
        }
      }
    }
    for (int j = 0; j < menu->board.width; ++j) {
      int val = menu->board.data[i][j];

      // Add spaces between the previously deleted columns
      if (menu->collapse_timer < 2) {
        for (int k = 0; k < menu->collapse_col_len; ++k)  {
          if (j == menu->collapse_col[k]) {
            x += menu->grid_side + 8;
            if (menu->collapse_timer > 1) x -= t * (menu->grid_side + 8);
          }
        }
      }

      if (val) {
        DrawRectangle(x - 8, y - 8, menu->grid_side + 16, menu->grid_side + 16, WHITE);
        DrawRectangle(x, y, menu->grid_side, menu->grid_side, palette[val - 1]);
        char c[2] = { (char)('A' + val - 1) , '\0' };
        DrawText(c, x + 0.5f * (menu->grid_side - MeasureText(c, grid_text)),
                 y + grid_pad, grid_text, palette_dark[val - 1]);
      }
      x += menu->grid_side + 8;
    }
    y += menu->grid_side + 8;
  }
}

static void render_selection(GameMenu *menu) {
  if (menu->selecting) {
    const int cursor_width = menu->grid_side / 3;
    const float t = smoothstep(menu->selection_lerp);

    Index ps = tile_position(menu, menu->selection);
    float pt = menu->collapse_timer - 1;
    pt *= pt;
    if (menu->collapse_timer < 2) {
      for (int i = 0; i < menu->collapse_row_len; ++i) {
        if (menu->selection.y >= menu->collapse_row[i]) {
          ps.y += menu->grid_side + 8;
          if (menu->collapse_timer > 1) {
            ps.y -= pt * (menu->grid_side + 8);
          }
        }
      }

      for (int i = 0; i < menu->collapse_col_len; ++i) {
        if (menu->selection.x >= menu->collapse_col[i]) {
          ps.x += menu->grid_side + 8;
          if (menu->collapse_timer > 1) {
            ps.x -= pt * (menu->grid_side + 8);
          }
        }
      }
    }

    Color c = palette[menu->board.data[menu->selection.y][menu->selection.x] - 1];
    c.r = c.r * 2 / 3;
    c.g = c.g * 2 / 3;
    c.b = c.b * 2 / 3;

    const int x0 = ps.x - 4 - 4 * t;
    const int x1 = ps.x + 4 - 4 * t;

    const int x2 = ps.x + menu->grid_side - cursor_width - 4 + 4 * t;
    const int x3 = ps.x + menu->grid_side - 4 + 4 * t;

    const int y0 = ps.y - 4 - 4 * t;
    const int y2 = ps.y + menu->grid_side - cursor_width - 4 + 4 * t;
    const int y1 = ps.y + 4 - 4 * t;
    const int y3 = ps.y + menu->grid_side - 4 + 4 * t;

    DrawRectangle(x0, y0, 8, 8 + cursor_width, c);
    DrawRectangle(x1, y0, cursor_width, 8, c);

    DrawRectangle(x2, y0, 8 + cursor_width, 8, c);
    DrawRectangle(x3, y1, 8, cursor_width, c);

    DrawRectangle(x0, y2, 8, 8 + cursor_width, c);
    DrawRectangle(x1, y3, cursor_width, 8, c);

    DrawRectangle(x2, y3, 8 + cursor_width, 8, c);
    DrawRectangle(x3, y2, 8, cursor_width, c);
  }
}

static void render_path(GameMenu *menu) {
  const float center = menu->grid_side * 0.5f;

  float x = 2 * menu->path_lerp - 1;
  x *= x;

  Color path_color = palette[menu->path_val - 1];
  path_color.a = 255 - 255 * x;

  Index begin = menu->path.data[0];
  Index end = menu->path.data[menu->path.len - 1];

  Color end_color = menu->path_lerp < 0.5f ? palette[menu->path_val - 1] : path_color;
  Index begin_pos = tile_position(menu, begin);
  Index end_pos = tile_position(menu, end);
  float r = menu->grid_side * 0.125f;

  for (int i = 1; i < menu->path.len - 1; ++i) {
    Index pos = tile_position(menu, menu->path.data[i]);
    DrawCircle(pos.x + center, pos.y + center, r, path_color);
  }

  for (int i = 1; i < menu->path.len; ++i) {
    Index c0 = tile_position(menu, menu->path.data[i - 1]);
    Index c1 = tile_position(menu, menu->path.data[i]);

    DrawLineEx((Vector2) {c0.x + center, c0.y + center}, (Vector2) {c1.x + center, c1.y + center},
               menu->grid_side * 0.12, path_color);
  }

  Color text_color = palette_dark[menu->path_val - 1];
  if (menu->path_lerp >= 0.5f) text_color.a = 255 - 255 * x;

  DrawRectangle(begin_pos.x, begin_pos.y, menu->grid_side, menu->grid_side, end_color);
  DrawRectangle(end_pos.x, end_pos.y, menu->grid_side, menu->grid_side, end_color);

  const int grid_text = menu->grid_side * 0.6;
  const int grid_pad = menu->grid_side * 0.2;
  char c[2] = { (char)('A' + menu->path_val - 1) , '\0' };
  DrawText(c, begin_pos.x + 0.5f * (menu->grid_side - MeasureText(c, grid_text)),
           begin_pos.y + grid_pad, grid_text, text_color);

  DrawText(c, end_pos.x + 0.5f * (menu->grid_side - MeasureText(c, grid_text)),
           end_pos.y + grid_pad, grid_text, text_color);
}

static void render_tutorial(GameMenu *menu) {
  if (!menu->is_tutorial) return;

  if (menu->tutorial.timer < TUTORIAL_TRANSITION) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, (uint8_t)(192 * (1 - menu->tutorial.timer / TUTORIAL_TRANSITION)) });
  } else if (menu->tutorial.stage < STAGE_FINAL) {
    if (menu->tutorial.timer < TUTORIAL_COOLDOWN) {
      if (menu->tutorial.timer >= TUTORIAL_COOLDOWN - TUTORIAL_TRANSITION) {
        float t = (menu->tutorial.timer + TUTORIAL_TRANSITION - TUTORIAL_COOLDOWN) / TUTORIAL_TRANSITION;
        if (t > 1) t = 1;
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, (uint8_t)(208 * t) });
      }
    } else {
      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, 208 });
      int w = GetScreenWidth();
      int center = GetScreenHeight() / 2;

      if (menu->tutorial.stage < STAGE_OTHER) {
        const char msg[] = "Press ENTER to continue";
        DrawText(msg, (w - MeasureText(msg, 32)) / 2, center + 64, 32, LIGHTGRAY);
      }

      switch (menu->tutorial.stage) {
        case STAGE_MOVE: {
          const char msg[] = "WASD or ARROWS to move";
          DrawText(msg, (w - MeasureText(msg, 48)) / 2, center - 64, 48, LIGHTGRAY);
        } break;
        case STAGE_SELECT: {
          const char msg[] = "SPACE or ENTER to select a tile";
          DrawText(msg, (w - MeasureText(msg, 48)) / 2, center - 64, 48, LIGHTGRAY);
        } break;
        case STAGE_CONNECT: {
          {
            const char msg[] = "Connect two matching tiles";
            DrawText(msg, (w - MeasureText(msg, 48)) / 2, center - 128, 48, LIGHTGRAY);
          }
          {
            const char msg[] = "with THREE or less lines";
            DrawText(msg, (w - MeasureText(msg, 48)) / 2, center - 64, 48, LIGHTGRAY);
          }
        } break;
        case STAGE_OTHER: {
          {
            const char msg[] = "Press E for a hint (no point)";
            DrawText(msg, (w - MeasureText(msg, 48)) / 2, center - 128, 48, LIGHTGRAY);
          }
          {
            const char msg[] = "Press Q to to back to the menu";
            DrawText(msg, (w - MeasureText(msg, 48)) / 2, center - 64, 48, LIGHTGRAY);
          }
          {
            const char msg[] = "You completed the tutorial. Try to finish the game!";
            DrawText(msg, (w - MeasureText(msg, 32)) / 2, center + 64, 32, LIGHTGRAY);
          }
          {
            const char msg[] = "Press ENTER to continue";
            DrawText(msg, (w - MeasureText(msg, 32)) / 2, center + 112, 32, LIGHTGRAY);
          }
        } break;
        case STAGE_FINAL: break;
      }
    }
  }
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

  menu->hinting = false;
  menu->hint_timer = 0;

  menu->collapse_col_len = 0;
  menu->collapse_row_len = 0;

  LevelConfig config = configs[game->config.level];

  menu->remaining = config.width * config.height;

  uint64_t *rng = &game->current_user->info.random_state;

  GameBoard_init(&menu->board, config.width, config.height, config.num_classes, rng);

  Queue_init(&menu->search_queue);

  update_size(menu, menu->board.width, menu->board.height);

  const float TAU = 6.283185307179586f;

  float side = pcg32_expo(rng);
  for (int i = 0; i < BG_SQUARE_COUNT; ++i) {
    menu->background_squares[i].side = side;
    side += pcg32_expo(rng);
  }

  for (int i = 0; i < BG_SQUARE_COUNT; ++i) {
    BackgroundSquare *square = menu->background_squares + i;
    float norm_side = 1 - (square->side / side);

    square->x = pcg32_uniform(rng);
    square->y = pcg32_uniform(rng);
    square->side = norm_side;
    square->col = pcg32_bounded(rng, config.num_classes);

    float freq_norm = 0.2f + 0.8f * norm_side;

    square->freq_x = (pcg32_uniform(rng) * 0.1f + 0.9f) / freq_norm;
    square->freq_y = (pcg32_uniform(rng) * 0.1f + 0.9f) / freq_norm;
    for (int i = 0; i < 5; ++i) {
      square->fbm_offset_x[i] = pcg32_uniform(rng) * TAU;
      square->fbm_offset_y[i] = pcg32_uniform(rng) * TAU;
    }
  }

  menu->background_timer = 0;

  menu->is_tutorial = !game->current_user->info.passed_tutorial;
  if (menu->is_tutorial) {
    menu->tutorial.stage = STAGE_MOVE;
    menu->tutorial.timer = TUTORIAL_COOLDOWN - TUTORIAL_TRANSITION;
  }
}

Scene Scene_game_update(Game *game, float dt) {
  (void)dt;
  GameMenu *menu = &game->menu.game;


  update_suggest(game);
  if (IsKeyPressed(KEY_Q)) if (!menu->is_tutorial || menu->tutorial.stage == STAGE_FINAL) return SCENE_HOME;

  if (!menu->is_tutorial || menu->tutorial.stage >= STAGE_FINAL || menu->tutorial.timer < TUTORIAL_COOLDOWN) {
    update_move(game, dt);
    switch (update_select(game)) {
      case STATUS_PLAYING: break;
      case STATUS_WON: return SCENE_WON;
      case STATUS_LOST: return SCENE_LOST;
    }
  } else {
    if (IsKeyPressed(KEY_ENTER)) {
      PlaySound(game->sounds[SOUND_SELECT]);
      menu->tutorial.stage = (TutorialStage)(menu->tutorial.stage + 1);
      menu->tutorial.timer = 0;
      if (menu->tutorial.stage == STAGE_FINAL) {
        game->current_user->info.passed_tutorial = true;
      }
    }
  }

  update_interpolation(menu, dt);

  update_size(menu, menu->board.width, menu->board.height);

  render_background(menu);
  render_board(menu);

  render_position(menu);

  if (menu->hinting) {
    for (int i = 0; i < 2; ++i) {
      const uint8_t alpha = 72 + 72 * sinf(5 * menu->hint_timer);
      Color hint_color = { 255, 255, 255, alpha };
      Index pos = tile_position(menu, menu->hint_indices[i]);
      DrawRectangle(pos.x, pos.y, menu->grid_side, menu->grid_side, hint_color);
    }
  }

  render_selection(menu);

  if (menu->path.len) {
    render_path(menu);
  }

  DrawRectangle(0, 0, GetScreenWidth(), HEADER_HEIGHT, (Color) {142, 228, 255, 255});
  char msg[1024];
  snprintf(msg, sizeof(msg), "Level: %d", game->config.level + 1);
  DrawText(msg, 32, 32, 32, BLACK);

  snprintf(msg, sizeof(msg), "Score: %d", menu->score);
  DrawText(msg, GetScreenWidth() - MeasureText(msg, 32) - 32, 32, 32, BLACK);

  render_tutorial(menu);

  return SCENE_GAME;
}

void Scene_game_unload(Game *game) {
  GameMenu *menu = &game->menu.game;
  Queue_deinit(&menu->search_queue);
  GameBoard_deinit(&menu->board);
  (void)game;
}

