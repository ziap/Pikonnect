#include "scene.h"
#include "config.h"
#include <raylib.h>
#include <stdio.h>

static float ease_cubic(float t) {
  return t * t * (3.0f - 2.0f * t);
}

Scene Scene_home_update(Game *game, float dt) {
  (void)game;
  (void)dt;

  const int HEADER_HEIGHT = 32 * 3;
  HomeMenu *menu = &game->home_menu;

  const float ANIMATION_SPEED = 5;

  if (IsKeyPressed(KEY_J) || IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_J) || IsKeyPressedRepeat(KEY_DOWN)) {
    menu->current_selection = (Selection)((menu->current_selection + 1) % SELECTION_LEN);
  }

  if (IsKeyPressed(KEY_K) || IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_K) || IsKeyPressedRepeat(KEY_UP)) {
    menu->current_selection = (Selection)((menu->current_selection + SELECTION_LEN - 1) % SELECTION_LEN);
  }

  if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
    if (IsKeyPressed(KEY_N) || IsKeyPressedRepeat(KEY_N)) {
      menu->current_selection = (Selection)((menu->current_selection + 1) % SELECTION_LEN);
    }

    if (IsKeyPressed(KEY_P) || IsKeyPressedRepeat(KEY_P)) {
      menu->current_selection = (Selection)((menu->current_selection + SELECTION_LEN - 1) % SELECTION_LEN);
    }
  }

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

  const int t = (SCREEN_WIDTH - 64) / 3;

  int selection_y = HEADER_HEIGHT + (REMAINING_HEIGHT - SELECTION_HEIGHT * SELECTION_LEN - 32 * (SELECTION_LEN - 1)) / 2;
  for (int i = 0; i < SELECTION_LEN; ++i) {
    const int offset = t - (t * 0.3f) * ease_cubic(menu->selection_lerp[i]);
    DrawRectangle(32 + offset, selection_y, (SCREEN_WIDTH - 32) - offset, SELECTION_HEIGHT, SKYBLUE);
    const char *txt = menu_names[i];
    DrawText(txt, 192 + offset, selection_y + 32, SELECTION_SIZE, BLACK);
    selection_y += SELECTION_HEIGHT + 32;
  }

  float *lerp = menu->selection_lerp + menu->current_selection;
  *lerp = *lerp < 1 ? *lerp + ANIMATION_SPEED * dt : 1;
  for (int i = 0; i < menu->current_selection; ++i) {
    menu->selection_lerp[i] = menu->selection_lerp[i] > 0 ? menu->selection_lerp[i] - ANIMATION_SPEED * dt : 0;
  }
  for (int i = menu->current_selection + 1; i < SELECTION_LEN; ++i) {
    menu->selection_lerp[i] = menu->selection_lerp[i] > 0 ? menu->selection_lerp[i] - ANIMATION_SPEED * dt : 0;
  }

  return SCENE_HOME;
}

