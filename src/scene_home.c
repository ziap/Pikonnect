#include "scene_home.h"

#include "utils.h"
#include "controls_menu.h"

void Scene_home_load(Game *game) {
  HomeMenu *menu = &game->menu.home;

  menu->current_selection = (HomeSelection)0;
  for (int i = 0; i < SELECTION_LEN; ++i) {
    menu->selection_lerp[i] = 0;
  }
}

Scene Scene_home_update(Game *game, float dt) {
  HomeMenu *menu = &game->menu.home;

  if (!game->current_user->info.passed_tutorial) {
    game->config.level = 0;
    game->config.gamemode = GAMEMODE_CLASSIC;

    return SCENE_GAME;
  }

  if (ControlsMenu_down() || ControlsMenu_tab_next()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    menu->current_selection = (HomeSelection)((menu->current_selection + 1) % SELECTION_LEN);
  }

  if (ControlsMenu_up() || ControlsMenu_tab_prev()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    menu->current_selection = (HomeSelection)((menu->current_selection + SELECTION_LEN - 1) % SELECTION_LEN);
  }

  if (ControlsMenu_confirm()) {
    PlaySound(game->sounds[SOUND_SELECT]);
    switch (menu->current_selection) {
      case SELECTION_CLASSIC: {
        game->config.gamemode = GAMEMODE_CLASSIC;
        return SCENE_LEVELS;
      } break;
      case SELECTION_COLLAPSE: {
        game->config.gamemode = GAMEMODE_COLLAPSE;
        return SCENE_LEVELS;
      } break;
      case SELECTION_LEADERBOARD: {
        return SCENE_LEADERBOARD;
      } break;
      case SELECTION_LEN: unreachable;
    }
  }

  const float anim_speed = 5;
  float *lerp = menu->selection_lerp + menu->current_selection;
  *lerp = *lerp < 1 ? *lerp + anim_speed * dt : 1;
  for (HomeSelection i = 0; i < menu->current_selection; ++i) {
    lerp = menu->selection_lerp + i;
    *lerp = *lerp > 0 ? *lerp - anim_speed * dt : 0;
  }
  for (int i = menu->current_selection + 1; i < SELECTION_LEN; ++i) {
    lerp = menu->selection_lerp + i;
    *lerp = *lerp > 0 ? *lerp - anim_speed * dt : 0;
  }

  Scene_draw_header(game, "Home");

  const char menu_names[3][12] = {
    "Classic", "Collapse", "Leaderboard"
  };

  const Color menu_colors[3] = {
    { 117, 222, 254, 255 },
    { 142, 137, 198, 255 },
    { 245, 123,  87, 255 },
  };

  const int stride = 112 + (GetScreenHeight() - HEADER_HEIGHT) / 20;
  const int offset_full = (GetScreenWidth() - 64) * 2 / 5;

  int y = (GetScreenHeight() + HEADER_HEIGHT + 32 - stride * SELECTION_LEN) / 2;
  for (int i = 0; i < SELECTION_LEN; ++i) {
    const int offset = offset_full - 128 * smoothstep(menu->selection_lerp[i]);
    DrawRectangle(32 + offset, y, (GetScreenWidth() - 32) - offset, 112, menu_colors[i]);
    const char *txt = menu_names[i];
    DrawText(txt, 192 + offset, y + 32, 48, BLACK);
    y += stride;
  }

  return SCENE_HOME;
}

