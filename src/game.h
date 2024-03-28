#ifndef GAME_H
#define GAME_H

#include "user.h"

struct LoginTextBoxData {
  char *input;
  int len;
  int cap;
};

enum LoginTextBox {
  TEXTBOX_USERNAME,
  TEXTBOX_PASSWORD,
  TEXTBOX_LEN
};

struct LoginMenu {
  LoginTextBox selected_textbox;
  char message[256];
  LoginTextBoxData textboxes[TEXTBOX_LEN];
};

enum HomeSelection {
  SELECTION_CLASSIC,
  SELECTION_COLLAPSE,
  SELECTION_LEADERBOARD,
  SELECTION_LEN
};

struct HomeMenu {
  HomeSelection current_selection;
  float selection_lerp[SELECTION_LEN];
};

struct LevelsMenu {
  int current_selection; 
  float selection_lerp[LEVEL_COUNT];
};

enum GameMode {
  GAMEMODE_CLASSIC,
  GAMEMODE_COLLAPSE,
};

struct Game {
  UserTable users;
  LoginMenu login_menu;
  HomeMenu home_menu;
  LevelsMenu level_menu;
  User *current_user;

  GameMode gamemode;
  int level;
};

extern void Game_init(Game *game);
extern void Game_deinit(Game *game);

#endif
