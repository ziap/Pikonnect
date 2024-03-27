#include "scene.h"
#include "config.h"
#include <raylib.h>

// Smoothstep function: https://en.wikipedia.org/wiki/Smoothstep
static float ease_cubic(float t) {
  return t * t * (3.0f - 2.0f * t);
}

static bool is_pressing_down() {
  if (IsKeyPressed(KEY_J) || IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_J) || IsKeyPressedRepeat(KEY_DOWN)) {
    return true;
  }

  if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
    if (IsKeyPressed(KEY_N) || IsKeyPressedRepeat(KEY_N)) {
      return true;
    }
  }

  if (!IsKeyDown(KEY_LEFT_SHIFT) && !IsKeyDown(KEY_RIGHT_SHIFT)) {
    if (IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) return true;
  }

  return false;
}

static bool is_pressing_up() {
  if (IsKeyPressed(KEY_K) || IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_K) || IsKeyPressedRepeat(KEY_UP)) {
    return true;
  }

  if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
    if (IsKeyPressed(KEY_P) || IsKeyPressedRepeat(KEY_P)) {
      return true;
    }
  }

  if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
    if (IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) return true;
  }

  return false;
}

Scene Scene_home_update(Game *game, float dt) {
  HomeMenu *menu = &game->home_menu;

  const float ANIMATION_SPEED = 5;

  if (is_pressing_down()) {
    menu->current_selection = (Selection)((menu->current_selection + 1) % SELECTION_LEN);
  }

  if (is_pressing_up()) {
    menu->current_selection = (Selection)((menu->current_selection + SELECTION_LEN - 1) % SELECTION_LEN);
  }

  if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressedRepeat(KEY_ENTER) || IsKeyPressedRepeat(KEY_SPACE)) {
    switch (menu->current_selection) {
      case SELECTION_CLASSIC: {
        game->gamemode = GAMEMODE_CLASSIC;
        return SCENE_LEVELS;
      } break;
      case SELECTION_COLLAPSE: {
        game->gamemode = GAMEMODE_COLLAPSE;
        return SCENE_LEVELS;
      } break;
      case SELECTION_LEADERBOARD: {
        return SCENE_LEADERBOARD;
      } break;
      case SELECTION_LEN: __builtin_unreachable(); break;
    }
  }

  Scene_draw_header(game, "Home");

  const int REMAINING_HEIGHT = SCREEN_HEIGHT - HEADER_HEIGHT;

  const char menu_names[3][12] = {
    "Classic", "Collapse", "Leaderboard"
  };

  const Color menu_colors[3] = {
    GetColor(0x38bdf8ff),
    GetColor(0xa3e635ff),
    GetColor(0xfacc15ff),
  };

  const int t = (SCREEN_WIDTH - 64) / 3;

  int selection_y = HEADER_HEIGHT + (REMAINING_HEIGHT - 112 * SELECTION_LEN - 32 * (SELECTION_LEN - 1)) / 2;
  for (int i = 0; i < SELECTION_LEN; ++i) {
    const int offset = t - (t * 0.3f) * ease_cubic(menu->selection_lerp[i]);
    DrawRectangle(32 + offset, selection_y, (SCREEN_WIDTH - 32) - offset, 112, menu_colors[i]);
    const char *txt = menu_names[i];
    DrawText(txt, 192 + offset, selection_y + 32, 48, BLACK);
    selection_y += 144;
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

