#include "scene.h"
#include "config.h"

#include <raylib.h>

Scene Scene_levels_update(Game *game, float dt) {
  (void)dt;

  Scene_draw_header(game, "Select level");
  
  const int GRID_SIDE = (SCREEN_WIDTH - 32 * 2 - 96 * 2) / 3;
  const int TEXT_SIZE = GRID_SIDE - 128;

  const int GRID_ROWS = (LEVEL_COUNT + 2) / 3;
  const int GRID_OFF = (SCREEN_HEIGHT - HEADER_HEIGHT - (GRID_SIDE * GRID_ROWS + 32 * (GRID_ROWS - 1))) / 2;

  const int x0 = 96;
  const int y0 = HEADER_HEIGHT + GRID_OFF;

  const Color grid_colors[2] = { LIGHTGRAY, GetColor(0xa3e635ff) };
  const Color text_colors[2] = { DARKGRAY, BLACK };

  DrawRectangle(x0, y0, GRID_SIDE, GRID_SIDE, grid_colors[1]);
  DrawText("1", x0 + (TEXT_SIZE - MeasureText("1", TEXT_SIZE)), y0 + 64, TEXT_SIZE, text_colors[1]);
  
  for (int i = 1; i < LEVEL_COUNT; ++i) {
    int x = x0 + (GRID_SIDE + 32) * (i % 3);
    int y = y0 + (GRID_SIDE + 32) * (i / 3);

    uint32_t solve_time = game->current_user->solve_time[i - 1];

    DrawRectangle(x, y, GRID_SIDE, GRID_SIDE, grid_colors[solve_time != NOT_SOLVED]);
    char c[2] = { (char)('1' + i), '\0'};

    DrawText(c, x + (GRID_SIDE - MeasureText(c, TEXT_SIZE)) / 2, y + 64, TEXT_SIZE, text_colors[solve_time != NOT_SOLVED]);
  }

  return SCENE_LEVELS;
}
