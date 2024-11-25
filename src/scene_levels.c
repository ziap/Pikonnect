#include "scene_levels.h"

#include "controls_menu.h"

void Scene_levels_load(Game *game) {
  LevelsMenu *menu = &game->menu.level;
  menu->current_selection = 0;

  for (int i = 0; i < LEVEL_COUNT; ++i) {
    menu->selection_lerp[i] = 0;
  }

  menu->grid_rows = (LEVEL_COUNT + 2) / 3;
  menu->grid_total = menu->grid_rows * 3;
}

Scene Scene_levels_update(Game *game, float dt) {
  LevelsMenu *menu = &game->menu.level;

  const int grid_side_x = (GetScreenWidth() - 256) / 3;
  const int grid_side_y = (GetScreenHeight() - HEADER_HEIGHT - 192 - (menu->grid_rows - 1) * 32) / menu->grid_rows;
  const int grid_side = grid_side_y < grid_side_x ? grid_side_y : grid_side_x;
  const int grid_textsize = grid_side * 6 / 10;
  const int grid_pad = grid_side * 2 / 10;

  uint32_t *selection = &menu->current_selection;

  if (ControlsMenu_up()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    *selection = (*selection + menu->grid_total - 3) % menu->grid_total;
  }
  if (ControlsMenu_down()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    *selection = (*selection + 3) % menu->grid_total;
  }
  if (*selection >= LEVEL_COUNT) {
    PlaySound(game->sounds[SOUND_CLICK]);
    *selection = LEVEL_COUNT - 1;
  }

  if (ControlsMenu_left() || ControlsMenu_tab_prev()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    *selection = (*selection + LEVEL_COUNT - 1) % LEVEL_COUNT;
  }
  if (ControlsMenu_right() || ControlsMenu_tab_next()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    *selection = (*selection + 1) % LEVEL_COUNT;
  }

  if (ControlsMenu_confirm()) {
    PlaySound(game->sounds[SOUND_SELECT]);
    menu->selection_lerp[*selection] = 0;

    if (*selection <= game->current_user->info.unlocked[game->config.gamemode]) {
      game->config.level = *selection;
      return SCENE_GAME;
    }
  }

  if (IsKeyPressed(KEY_Q) || IsKeyPressedRepeat(KEY_Q)) {
    PlaySound(game->sounds[SOUND_SELECT]);
    return SCENE_HOME;
  }

  const float anim_speed = 10;
  float *lerp = menu->selection_lerp + *selection;
  *lerp = *lerp < 1 ? *lerp + anim_speed * dt : 1;
  for (uint32_t i = 0; i < *selection; ++i) {
    lerp = menu->selection_lerp + i;
    *lerp = *lerp > 0 ? *lerp - anim_speed * dt : 0;
  }
  for (int i = *selection + 1; i < LEVEL_COUNT; ++i) {
    lerp = menu->selection_lerp + i;
    *lerp = *lerp > 0 ? *lerp - anim_speed * dt : 0;
  }


  Scene_draw_header(game, "Select level");

  const int x0 = (GetScreenWidth() - grid_side * 3 - 64) / 2;
  const int y0 = (GetScreenHeight() + HEADER_HEIGHT + 32 - (grid_side + 32) * menu->grid_rows) / 2;

  const Color grid_colors[2] = { LIGHTGRAY, { 0, 255, 157, 255 } };
  const Color text_colors[2] = { DARKGRAY, BLACK };

  uint32_t unlocked = game->current_user->info.unlocked[game->config.gamemode];

  for (uint32_t i = 0; i < LEVEL_COUNT; ++i) {
    int x = x0 + (grid_side + 32) * (i % 3);
    int y = y0 + (grid_side + 32) * (i / 3);

    float t = menu->selection_lerp[i];

    DrawRectangle(x - 8 * t, y - 8 * t, grid_side + 16 * t, grid_side + 16 * t, grid_colors[i <= unlocked]);
    char c[2] = { (char)('1' + i), '\0'};

    DrawText(c, x + (grid_side - MeasureText(c, grid_textsize)) * 0.5f - 8 * t, y + grid_pad - 8 * t, grid_textsize + 16 * t, text_colors[i <= unlocked]);
  }

  return SCENE_LEVELS;
}
