#include "menu_levels.h"

void LevelsMenu_init(LevelsMenu *menu) {
  menu->current_selection = 0;

  for (int i = 0; i < LEVEL_COUNT; ++i) {
    menu->selection_lerp[i] = 0.0;
  }
}
