#include "scene.h"
#include "config.h"
#include "controls_menu.h"

#include <raylib.h>

Scene Scene_levels_update(Game *game, float dt) {
  LevelsMenu *menu= &game->level_menu;

  const int GRID_ROWS = (LEVEL_COUNT + 2) / 3;
  const int GRID_TOTAL = GRID_ROWS * 3;
  const int GRID_SIDE = (SCREEN_WIDTH - 32 * 2 - 96 * 2) / 3;
  const int TEXT_SIZE = GRID_SIDE - 128;
  const int GRID_OFF = (SCREEN_HEIGHT - HEADER_HEIGHT - (GRID_SIDE * GRID_ROWS + 32 * (GRID_ROWS - 1))) / 2;

  int *selection = &menu->current_selection;

  if (ControlsMenu_up()) {
    *selection = (*selection + GRID_TOTAL - 3) % GRID_TOTAL;
  }
  if (ControlsMenu_down()) {
    *selection = (*selection + 3) % GRID_TOTAL;
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

    if (*selection == 0 || game->current_user->solve_time[*selection - 1] != NOT_SOLVED) {
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
  const int y0 = HEADER_HEIGHT + GRID_OFF;

  const Color grid_colors[2] = { LIGHTGRAY, GetColor(0xa3e635ff) };
  const Color text_colors[2] = { DARKGRAY, BLACK };


  float t0 = menu->selection_lerp[0];
  DrawRectangle(x0 - 16 * t0, y0 - 16 * t0, GRID_SIDE + 32 * t0, GRID_SIDE + 32 * t0, grid_colors[1]);
  DrawText("1", x0 + (TEXT_SIZE - MeasureText("1", TEXT_SIZE)) - 8 * t0, y0 + 64 - 8 * t0, TEXT_SIZE + 16 * t0, text_colors[1]);
  
  for (int i = 1; i < LEVEL_COUNT; ++i) {
    int x = x0 + (GRID_SIDE + 32) * (i % 3);
    int y = y0 + (GRID_SIDE + 32) * (i / 3);

    uint32_t solve_time = game->current_user->solve_time[i - 1];
    float t = menu->selection_lerp[i];

    DrawRectangle(x - 8 * t, y - 8 * t, GRID_SIDE + 16 * t, GRID_SIDE + 16 * t, grid_colors[solve_time != NOT_SOLVED]);
    char c[2] = { (char)('1' + i), '\0'};

    DrawText(c, x + (GRID_SIDE - MeasureText(c, TEXT_SIZE)) * 0.5f - 8 * t, y + 64 - 8 * t, TEXT_SIZE + 16 * t, text_colors[solve_time != NOT_SOLVED]);
  }
  
  return SCENE_LEVELS;
}
