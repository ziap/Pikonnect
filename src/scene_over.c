#include "scene_over.h"

#include <stdio.h>

#include "controls_menu.h"
#include "utils.h"

// A game over scene for both winning and losing.

static const Color accent_color = { 0, 135, 204, 255 };

static void draw_text_centered(const char* title, int y, int size, Color c) {
  DrawText(title, (GetScreenWidth() - MeasureText(title, size)) / 2, y, size, c);
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
    // Not the last level, draw the next level button and the main menu button
    // Also handle keyboard input for switching focus

    if (ControlsMenu_left() || ControlsMenu_tab_prev()) {
      PlaySound(game->sounds[SOUND_CLICK]);
      menu->selection = (WonBtns)((menu->selection + WON_BTN_LEN - 1) % WON_BTN_LEN);
    }

    if (ControlsMenu_right() || ControlsMenu_tab_next()) {
      PlaySound(game->sounds[SOUND_CLICK]);
      menu->selection = (WonBtns)((menu->selection + 1) % WON_BTN_LEN);
    }

    if (ControlsMenu_confirm()) {
      PlaySound(game->sounds[SOUND_SELECT]);
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
      [WON_BTN_NEXT] = "Next level",
      [WON_BTN_MENU] = "Main menu",
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
    int x = (GetScreenWidth() - total_width) / 2;
    for (WonBtns i = 0; i < WON_BTN_LEN; ++i) {
      Color c = menu->selection == i ? accent_color : GRAY;
      DrawText(btns[i], x, y, 32, c);
      x += text_width[i] + 64;
    }
  } else {
    // There's no next level, only draw the home menu button
    if (ControlsMenu_confirm()) {
      PlaySound(game->sounds[SOUND_SELECT]);
      return SCENE_HOME;
    }

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
  draw_text_centered("No valid move left, try again?", 318, 32, DARKGRAY);

  // Similar to the win scene with the next level button
  // We have to draw two buttons and handle keyboard events

  if (ControlsMenu_left() || ControlsMenu_tab_prev()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    menu->selection = (LostBtns)((menu->selection + LOST_BTN_LEN - 1) % LOST_BTN_LEN);
  }

  if (ControlsMenu_right() || ControlsMenu_tab_next()) {
    PlaySound(game->sounds[SOUND_CLICK]);
    menu->selection = (LostBtns)((menu->selection + 1) % LOST_BTN_LEN);
  }

  if (ControlsMenu_confirm()) {
    PlaySound(game->sounds[SOUND_SELECT]);
    switch (menu->selection) {
      case LOST_BTN_RESTART: return SCENE_GAME;
      case LOST_BTN_MENU: return SCENE_HOME;
      case LOST_BTN_LEN: unreachable;
    }
  }

  const char *btns[LOST_BTN_LEN] = {
    [LOST_BTN_RESTART] = "Retry level",
    [LOST_BTN_MENU] = "Main menu",
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
  int x = (GetScreenWidth() - total_width) / 2;
  for (LostBtns i = 0; i < LOST_BTN_LEN; ++i) {
    Color c = menu->selection == i ? accent_color : GRAY;
    DrawText(btns[i], x, y, 32, c);
    x += text_width[i] + 64;
  }
  return SCENE_LOST;
}
