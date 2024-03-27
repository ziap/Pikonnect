#include "menu_home.h"

void HomeMenu_init(HomeMenu *menu) {
  menu->current_selection = (Selection)0;

  for (int i = 0; i < SELECTION_LEN; ++i) {
    menu->selection_lerp[i] = 0.0f;
  }
};
