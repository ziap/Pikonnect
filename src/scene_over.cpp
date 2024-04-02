#include "scene_over.h"
#include "controls_menu.h"
#include "utils.h"

#include <raylib.h>
#include <stdio.h>

static const Color accent_color = { 0, 135, 204, 255 };

static void draw_text_centered(const char* title, int y, int size, Color c) {
  DrawText(title, (SCREEN_WIDTH - MeasureText(title, size)) / 2, y, size, c);
}

void Scene_won_load(Game *game) {
  WonMenu *menu = &game->menu.won;
  menu->selection = (WonBtns)0;
}

Scene Scene_won_update(Game *game, float dt) {
  (void)dt;
  WonMenu *menu = &game->menu.won;

  draw_text_centered("You won!", 150, 104, accent_color);
  char msg[1024];
  snprintf(msg, sizeof(msg), "Your score: %d%s", game->result.last_score,
           game->result.new_best ? " (New best!)" : "");

  draw_text_centered(msg, 318, 32, DARKGRAY);

  if (game->config.level + 1 < LEVEL_COUNT) {
    if (ControlsMenu_left() || ControlsMenu_tab_prev()) {
      menu->selection = (WonBtns)((menu->selection + WON_BTN_LEN - 1) % WON_BTN_LEN);
    }

    if (ControlsMenu_right() || ControlsMenu_tab_next()) {
      menu->selection = (WonBtns)((menu->selection + 1) % WON_BTN_LEN);
    }

    if (ControlsMenu_confirm()) {
      switch (menu->selection) {
        case WON_BTN_NEXT: {
          game->config.level++;
          return SCENE_GAME;
        }
        case WON_BTN_MENU: return SCENE_HOME;
        case WON_BTN_LEN: unreachable;
      }
    }

    const char *btns[WON_BTN_LEN] = {
      "Next level",
      "Main menu",
    };

    int text_width[WON_BTN_LEN];
    int total_width = 0;
    for (int i = 0; i < WON_BTN_LEN; ++i) {
      int w = MeasureText(btns[i], 32);
      text_width[i] = w;
      total_width += w;
    }

    total_width += 64 * (WON_BTN_LEN - 1);
    int y = 446;
    int x = (SCREEN_WIDTH - total_width) / 2;
    for (int i = 0; i < WON_BTN_LEN; ++i) {
      Color c = menu->selection == i ? accent_color : GRAY;
      DrawText(btns[i], x, y, 32, c);
      x += text_width[i] + 64;
    }
  } else {
    if (ControlsMenu_confirm()) return SCENE_HOME;

    const char *txt = "Main menu";
    draw_text_centered(txt, 446, 32, accent_color);
  }

  return SCENE_WON;
}

void Scene_lost_load(Game *game) {
  LostMenu *menu = &game->menu.lost;
  menu->selection = (LostBtns)0;
}

Scene Scene_lost_update(Game *game, float dt) {
  LostMenu *menu = &game->menu.lost;
  (void)dt;
  draw_text_centered("You lost!", 150, 104, accent_color);
  draw_text_centered("Do you want to try again?", 318, 32, DARKGRAY);

  if (ControlsMenu_left() || ControlsMenu_tab_prev()) {
    menu->selection = (LostBtns)((menu->selection + LOST_BTN_LEN - 1) % LOST_BTN_LEN);
  }

  if (ControlsMenu_right() || ControlsMenu_tab_next()) {
    menu->selection = (LostBtns)((menu->selection + 1) % LOST_BTN_LEN);
  }

  if (ControlsMenu_confirm()) {
    switch (menu->selection) {
      case LOST_BTN_RESTART: return SCENE_GAME;
      case LOST_BTN_MENU: return SCENE_HOME;
      case LOST_BTN_LEN: unreachable;
    }
  }

  const char *btns[LOST_BTN_LEN] = {
    "Retry level",
    "Main menu",
  };

  int text_width[LOST_BTN_LEN];
  int total_width = 0;
  for (int i = 0; i < LOST_BTN_LEN; ++i) {
    int w = MeasureText(btns[i], 32);
    text_width[i] = w;
    total_width += w;
  }

  total_width += 64 * (LOST_BTN_LEN - 1);
  int y = 446;
  int x = (SCREEN_WIDTH - total_width) / 2;
  for (int i = 0; i < LOST_BTN_LEN; ++i) {
    Color c = menu->selection == i ? accent_color : GRAY;
    DrawText(btns[i], x, y, 32, c);
    x += text_width[i] + 64;
  }
  return SCENE_LOST;
}
