#include "scene_game.h"

#include <math.h>
#include <stdio.h>

#include "utils.h"
#include "palette.h"
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

enum GameStatus {
  STATUS_PLAYING,
  STATUS_WON,
  STATUS_LOST
};

static Index tile_position(GameMenu *menu, Index tile) {
  return {
    menu->y0 + (menu->grid_side + 8) * tile.y,
    menu->x0 + (menu->grid_side + 8) * tile.x,
  };
}

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

  const float side_x = (SCREEN_WIDTH - gap_x) / (w + 2);
  const float side_y = (SCREEN_HEIGHT - HEADER_HEIGHT - gap_y) / (h + 2);

  menu->grid_side = side_x < side_y ? side_x : side_y;
  if (menu->grid_side > 96) menu->grid_side = 96;

  menu->x0 = (SCREEN_WIDTH - menu->grid_side * w - gap_x) / 2;
  menu->y0 = (SCREEN_HEIGHT + HEADER_HEIGHT - menu->grid_side * h - gap_y) / 2;
}

static GameStatus remove_pair(Game *game, Path path) {
  Index p1 = path.data[0];
  Index p2 = path.data[path.len - 1];
  GameMenu *menu = &game->menu.game;

  *GameBoard_index(menu->board, p1) = 0;
  *GameBoard_index(menu->board, p2) = 0;
  menu->remaining -= 2;

  if (game->config.gamemode == GAMEMODE_COLLAPSE) {
    bool remove_second_x = p2.x != p1.x;
    bool remove_second_y = p2.y != p1.y;
    menu->collapse_row_len = 0;
    menu->collapse_col_len = 0;
    menu->collapse_timer = 0;

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
// f(t) = (c0 * t^5 + c1 * t^4 + c2 * t^3 + c3 * t^2 + c4 * t + c5) * s
//
// f(0) = 10 * s
// f(10) = s
// f'(0) = f'(10) = f''(0) = f''(10) = 0
static int score(float t, int s) {
  if (t > 10) return s;
  return (int)((500000 - ((27 * t - 675) * t + 4500) * t * t * t) * s + 25000) / 50000;
}

static void update_move(Game *game, float dt) {
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

      PlaySound(game->sounds[SOUND_CLICK]);
      next_position = Index_step(next_position, (Dir)dir);
    } else if (pressing[dir]) {
      if (!pressing[opposite[dir]]) {
        menu->move_direction = (Dir)dir;
      }

      if (menu->move_direction == dir) {
        menu->as_delay[dir] -= dt;
        if (menu->as_delay[dir] <= 0) {
          menu->as_delay[dir] = ARR;
          PlaySound(game->sounds[SOUND_CLICK]);
          next_position = Index_step(next_position, (Dir)dir);
        }
      }
    }
  }

  menu->pos = next_position;
  if (menu->pos.x < 0) menu->pos.x = 0;
  if (menu->pos.y < 0) menu->pos.y = 0;

  if (menu->pos.x >= menu->board.width) menu->pos.x = menu->board.width - 1;
  if (menu->pos.y >= menu->board.height) menu->pos.y = menu->board.height - 1;
}

static GameStatus update_select(Game *game) {
  GameMenu *menu = &game->menu.game;

  if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
    if (*GameBoard_index(menu->board, menu->pos)) {
      if (!menu->selecting) {
        PlaySound(game->sounds[SOUND_SELECT]);
        menu->selecting = true;
        menu->selection = menu->pos;
        menu->selection_lerp = 1;
      } else {
        Path path = Search_valid_path(menu->board, menu->pos, menu->selection, &menu->search_queue);

        if (path.len > 0) {
          PlaySound(game->sounds[SOUND_CORRECT]);
          menu->selecting = false;
          menu->path_val = *GameBoard_index(menu->board, menu->pos);
        
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
          PlaySound(game->sounds[SOUND_SELECT]);
          menu->selection = menu->pos;
          menu->selection_lerp = 1;
        }
      }
    }
  }

  return STATUS_PLAYING;
}

static GameStatus update_suggest(Game *game) {
  GameMenu *menu = &game->menu.game;

  if (IsKeyPressed(KEY_X)) {
    PlaySound(game->sounds[SOUND_SELECT]);
    Path path = Search_suggest_move(menu->board, &menu->search_queue);
    if (path.len >= 2) {
      menu->hinting = true;
      menu->hint_timer = 0;
      menu->hint_indices[0] = path.data[0];
      menu->hint_indices[1] = path.data[path.len - 1];
    }
  }

  return STATUS_PLAYING;
}

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
    if (menu->collapse_timer < 2) {
      for (int j = 0; j < menu->collapse_row_len; ++j)  {
        if (i == menu->collapse_row[j]) {
          y += menu->grid_side + 8;
          if (menu->collapse_timer > 1) y -= t * (menu->grid_side + 8);
        } 
      }
    }
    for (int j = 0; j < menu->board.width; ++j) {
      int val = *GameBoard_index(menu->board, {i, j});
      
      if (menu->collapse_timer < 2) {
        for (int k = 0; k < menu->collapse_col_len; ++k)  {
          if (j == menu->collapse_col[k]) {
            x += menu->grid_side + 8;
            if (menu->collapse_timer > 1) x -= t * (menu->grid_side + 8);
          }
        }
      }

      if (val) {
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

    Color c = palette[*GameBoard_index(menu->board, menu->selection) - 1];
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

void render_path(GameMenu *menu) {
  const float center = menu->grid_side * 0.5f;

  float x = 2 * menu->path_lerp - 1;
  x *= x;

  Color path_color = palette[menu->path_val - 1];
  path_color.g += (255 - path_color.g) * x;
  path_color.r += (255 - path_color.r) * x;
  path_color.b += (255 - path_color.b) * x;

  Index begin = menu->path.data[0];
  Index end = menu->path.data[menu->path.len - 1];

  Color end_color = menu->path_lerp < 0.5f ? palette[menu->path_val - 1] : path_color;
  Index begin_pos = tile_position(menu, begin);
  Index end_pos = tile_position(menu, end);

  for (int i = 1; i < menu->path.len - 1; ++i) {
    Index pos = tile_position(menu, menu->path.data[i]);
    DrawCircle(pos.x + center, pos.y + center, menu->grid_side * 0.125f,
               path_color);
  }

  for (int i = 1; i < menu->path.len; ++i) {
    Index c0 = tile_position(menu, menu->path.data[i - 1]);
    Index c1 = tile_position(menu, menu->path.data[i]);

    DrawLineEx({c0.x + center, c0.y + center}, {c1.x + center, c1.y + center},
               menu->grid_side * 0.12, path_color);
  }

  Color text_color = palette_dark[menu->path_val - 1];
  if (menu->path_lerp >= 0.5f) {
    text_color.g += (255 - text_color.g) * x;
    text_color.r += (255 - text_color.r) * x;
    text_color.b += (255 - text_color.b) * x;
  }

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

  GameBoard_init(&menu->board, config.width, config.height, config.num_classes,
                 &game->current_user->info.random_state);

  Queue_init(&menu->search_queue);

  update_size(menu, menu->board.width, menu->board.height);
}

Scene Scene_game_update(Game *game, float dt) {
  (void)dt;
  GameMenu *menu = &game->menu.game;

  update_move(game, dt);
  switch (update_select(game)) {
    case STATUS_PLAYING: break;
    case STATUS_WON: return SCENE_WON;
    case STATUS_LOST: return SCENE_LOST;
  }

  switch (update_suggest(game)) {
    case STATUS_PLAYING: break;
    case STATUS_WON: return SCENE_WON;
    case STATUS_LOST: return SCENE_LOST;
  }

  if (IsKeyPressed(KEY_Q)) return SCENE_HOME;

  update_interpolation(menu, dt);

  update_size(menu, menu->board.width, menu->board.height);

  render_position(menu);
  render_board(menu);

  if (menu->hinting) {
    for (int i = 0; i < 2; ++i) {
      const uint8_t alpha = 72 + 72 * sin(5 * menu->hint_timer);
      Color hint_color = { 255, 255, 255, alpha };
      Index pos = tile_position(menu, menu->hint_indices[i]);
      DrawRectangle(pos.x, pos.y, menu->grid_side, menu->grid_side, hint_color);
    }
  }

  render_selection(menu);

  if (menu->path.len) {
    render_path(menu);
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
  Queue_deinit(&menu->search_queue);
  GameBoard_deinit(&menu->board);
  (void)game;
}

