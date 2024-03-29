#include "scene_levels.h"
#include "controls_menu.h"

#include <raylib.h>

void Scene_levels_load(Game *game) {
  LevelsMenu *menu= &game->level_menu;
  menu->current_selection = 0;

  for (int i = 0; i < LEVEL_COUNT; ++i) {
    menu->selection_lerp[i] = 0;
  }
}

Scene Scene_levels_update(Game *game, float dt) {
  LevelsMenu *menu= &game->level_menu;

  const int grid_rows = (LEVEL_COUNT + 2) / 3;
  const int grid_total = grid_rows * 3;
  const int grid_side = (SCREEN_WIDTH - 32 * 2 - 96 * 2) / 3;
  const int grid_textsize = grid_side - 128;
  const int grid_off = (SCREEN_HEIGHT - HEADER_HEIGHT - (grid_side * grid_rows + 32 * (grid_rows - 1))) / 2;

  int *selection = &menu->current_selection;

  if (ControlsMenu_up()) {
    *selection = (*selection + grid_total - 3) % grid_total;
  }
  if (ControlsMenu_down()) {
    *selection = (*selection + 3) % grid_total;
  }
  if (*selection >= LEVEL_COUNT) {
    *selection = LEVEL_COUNT - 1;
  }

  if (ControlsMenu_left() || ControlsMenu_tab_prev()) {
    *selection = (*selection + LEVEL_COUNT - 1) % LEVEL_COUNT;
  }
  if (ControlsMenu_right() || ControlsMenu_tab_next()) {
    *selection = (*selection + 1) % LEVEL_COUNT;
  }

  if (ControlsMenu_confirm()) {
    menu->selection_lerp[*selection] = 0;

    if (*selection == 0 || game->current_user->info.solve_time[*selection - 1] != NOT_SOLVED) {
      game->level = *selection;
      return SCENE_GAME;
    }
  }

  const float ANIMATION_SPEED = 10;
  float *lerp = menu->selection_lerp + *selection;
  *lerp = *lerp < 1 ? *lerp + ANIMATION_SPEED * dt : 1;
  for (int i = 0; i < *selection; ++i) {
    lerp = menu->selection_lerp + i;
    *lerp = *lerp > 0 ? *lerp - ANIMATION_SPEED * dt : 0;
  }
  for (int i = *selection + 1; i < LEVEL_COUNT; ++i) {
    lerp = menu->selection_lerp + i;
    *lerp = *lerp > 0 ? *lerp - ANIMATION_SPEED * dt : 0;
  }


  Scene_draw_header(game, "Select level");
  
  const int x0 = 96;
  const int y0 = HEADER_HEIGHT + grid_off;

  const Color grid_colors[2] = { LIGHTGRAY, GetColor(0xa3e635ff) };
  const Color text_colors[2] = { DARKGRAY, BLACK };


  float t0 = menu->selection_lerp[0];
  DrawRectangle(x0 - 16 * t0, y0 - 16 * t0, grid_side + 32 * t0, grid_side + 32 * t0, grid_colors[1]);
  DrawText("1", x0 + (grid_textsize - MeasureText("1", grid_textsize)) - 8 * t0, y0 + 64 - 8 * t0, grid_textsize + 16 * t0, text_colors[1]);
  
  for (int i = 1; i < LEVEL_COUNT; ++i) {
    int x = x0 + (grid_side + 32) * (i % 3);
    int y = y0 + (grid_side + 32) * (i / 3);

    uint32_t solve_time = game->current_user->info.solve_time[i - 1];
    float t = menu->selection_lerp[i];

    DrawRectangle(x - 8 * t, y - 8 * t, grid_side + 16 * t, grid_side + 16 * t, grid_colors[solve_time != NOT_SOLVED]);
    char c[2] = { (char)('1' + i), '\0'};

    DrawText(c, x + (grid_side - MeasureText(c, grid_textsize)) * 0.5f - 8 * t, y + 64 - 8 * t, grid_textsize + 16 * t, text_colors[solve_time != NOT_SOLVED]);
  }
  
  return SCENE_LEVELS;
}