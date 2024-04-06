#include "scene_leaderboard.h"

#include <stdio.h>
#include <raylib.h>

#include "palette.h"

static void insert_leaderboard(User **list, User *user, int pos) {
  int i = pos;
  uint32_t score = user->info.best_score;
  while (i > 0 && list[i - 1]->info.best_score < score) {
    list[i] = list[i - 1];
    --i;
  }
  list[i] = user;
}

void Scene_leaderboard_load(Game *game) {
  LeaderboardMenu *menu = &game->menu.leaderboard;
  UserTable table = game->users;
  menu->top_len = table.len < 5 ? table.len : 5;
  
  for (uint32_t i = 0; i < menu->top_len; ++i) {
    insert_leaderboard(menu->top_users, table.data + i, i);
  }

  User **last = menu->top_users + menu->top_len - 1;
  for (uint32_t i = menu->top_len; i < table.len; ++i) {
    User *user = table.data + i;
    if (user->info.best_score > (*last)->info.best_score) {
      insert_leaderboard(menu->top_users, user, menu->top_len - 1);
    }
  }

  menu->index_width = MeasureText("0", 48);
}

Scene Scene_leaderboard_update(Game *game, float dt) {
  (void)dt;
  LeaderboardMenu *menu = &game->menu.leaderboard;

  if (IsKeyPressed(KEY_Q) || IsKeyPressedRepeat(KEY_Q)) return SCENE_HOME;

  int y = (SCREEN_HEIGHT + HEADER_HEIGHT - 112 * 5 + 16) / 2;
  int x = y - HEADER_HEIGHT;
  for (uint32_t i = 0; i < menu->top_len; ++i) {
    User *user = menu->top_users[i];
    Color banner = user == game->current_user ? palette[0] : LIGHTGRAY;
    DrawRectangle(x, y, SCREEN_WIDTH - 2 * x, 96, banner);
    char num[2] = { (char)(i + '1'), '\0' };
    DrawText(num, x + 24, y + 24, 48, DARKGRAY);
    DrawText(user->name, x + 72 + menu->index_width, y + 24, 48, BLACK);
    char score[1024];
    snprintf(score, sizeof(score), "%d", user->info.best_score);
    int x_txt = x + SCREEN_WIDTH - MeasureText(score, 48) - 2 * x - 24;
    DrawText(score, x_txt, y + 24, 48, DARKGRAY);
    y += 112;
  }

  Scene_draw_header(game, "Leaderboard");

  return SCENE_LEADERBOARD;
};
