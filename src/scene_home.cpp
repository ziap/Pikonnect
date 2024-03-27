#include "scene.h"
#include "config.h"
#include <raylib.h>

#include "controls_menu.h"

Scene Scene_home_update(Game *game, float dt) {
  HomeMenu *menu = &game->home_menu;

  if (ControlsMenu_down() || ControlsMenu_tab_next()) {
    menu->current_selection = (Selection)((menu->current_selection + 1) % SELECTION_LEN);
  }

  if (ControlsMenu_up() || ControlsMenu_tab_prev()) {
    menu->current_selection = (Selection)((menu->current_selection + SELECTION_LEN - 1) % SELECTION_LEN);
  }

  if (ControlsMenu_confirm()) {
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

  const float ANIMATION_SPEED = 5;
  float *lerp = menu->selection_lerp + menu->current_selection;
  *lerp = *lerp < 1 ? *lerp + ANIMATION_SPEED * dt : 1;
  for (int i = 0; i < menu->current_selection; ++i) {
    lerp = menu->selection_lerp + i;
    *lerp = *lerp > 0 ? *lerp - ANIMATION_SPEED * dt : 0;
  }
  for (int i = menu->current_selection + 1; i < SELECTION_LEN; ++i) {
    lerp = menu->selection_lerp + i;
    *lerp = *lerp > 0 ? *lerp - ANIMATION_SPEED * dt : 0;
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

  const int offset_full = (SCREEN_WIDTH - 64) / 3;

  int selection_y = HEADER_HEIGHT + (REMAINING_HEIGHT - 112 * SELECTION_LEN - 32 * (SELECTION_LEN - 1)) / 2;
  for (int i = 0; i < SELECTION_LEN; ++i) {
    const int offset = offset_full - (offset_full * 0.3f) * easing_cubic(menu->selection_lerp[i]);
    DrawRectangle(32 + offset, selection_y, (SCREEN_WIDTH - 32) - offset, 112, menu_colors[i]);
    const char *txt = menu_names[i];
    DrawText(txt, 192 + offset, selection_y + 32, 48, BLACK);
    selection_y += 144;
  }

  return SCENE_HOME;
}

