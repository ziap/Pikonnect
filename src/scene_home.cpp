#include "scene.h"
#include "config.h"
#include <raylib.h>
#include <stdio.h>

Scene Scene_home_update(Game *game, float dt) {
  (void)game;
  (void)dt;

  const int HEADER_HEIGHT = 32 * 3;

  // const int ANIMATION_TIME_MS = 300;

  DrawRectangle(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, LIGHTGRAY);
  DrawText("Home", 32, 32, 32, BLACK);

  char msg[1024];
  snprintf(msg, sizeof(msg), "Logged in as: %s", game->current_user->name);

  DrawText(msg, SCREEN_WIDTH - MeasureText(msg, 32) - 32, 32, 32, DARKGRAY);

  const int REMAINING_HEIGHT = SCREEN_HEIGHT - HEADER_HEIGHT;
  const int SELECTION_SIZE = 48;
  const int SELECTION_HEIGHT = SELECTION_SIZE + 2 * 32;

  const char menu_names[3][12] = {
    "Classic", "Collapse", "Leaderboard"
  };

  int selection_y = HEADER_HEIGHT + (REMAINING_HEIGHT - SELECTION_HEIGHT * SELECTION_LEN - 32 * (SELECTION_LEN - 1)) / 2;
  for (int i = 0; i < SELECTION_LEN; ++i) {
    DrawRectangle(32, selection_y, SCREEN_WIDTH - 64, SELECTION_HEIGHT, LIGHTGRAY);
    selection_y += SELECTION_HEIGHT + 32;
  }

  return SCENE_HOME;
}

