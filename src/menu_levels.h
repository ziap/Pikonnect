#ifndef MENU_LEVELS_H
#define MENU_LEVELS_H

#include "config.h"

struct LevelsMenu {
  int current_selection; 
  float selection_lerp[LEVEL_COUNT];
};

extern void LevelsMenu_init(LevelsMenu *menu);

#endif
