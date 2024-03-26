#ifndef HOME_H
#define HOME_H

#include <stdint.h>

enum Selection {
  SELECTION_CLASSIC,
  SELECTION_COLLAPSE,
  SELECTION_LEADERBOARD,
  SELECTION_LEN
};

struct HomeMenu {
  Selection current_selection;
  uint32_t selection_interpolation[SELECTION_LEN];
};

extern void HomeMenu_init(HomeMenu *menu);

#endif
