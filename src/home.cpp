#include "home.h"

void HomeMenu_init(HomeMenu *menu) {
  menu->current_selection = (Selection)0;
  menu->selection_interpolation[0] = -1;

  for (int i = 1; i < SELECTION_LEN; ++i) {
    menu->selection_interpolation[i] = 0;
  }
};
