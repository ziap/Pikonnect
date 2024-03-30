#ifndef GAME_H
#define GAME_H

#include "user.h"
#include "board_array.h"

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
  char message[64];
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

struct GameConfig {
  GameMode gamemode;
  int level;
};

struct GameMenu {
  uint32_t start_time;
  uint64_t random_state;
  GameBoard board;
};

struct Game {
  UserTable users;
  User *current_user;
  GameConfig config;

  union {
    LoginMenu login;
    HomeMenu home;
    LevelsMenu level;
    GameMenu game;
  } menu;
};

extern void Game_init(Game *game);
extern void Game_deinit(Game *game);

#endif
