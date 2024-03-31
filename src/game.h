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

enum Dir {
  DIR_UP,
  DIR_LEFT,
  DIR_DOWN,
  DIR_RIGHT,
  DIR_LEN
};

static const Index next_index[DIR_LEN] = {
  {-1, 0},
  {0, -1},
  {1, 0},
  {0, 1}
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
  GameBoard board;

  Index position;

  float as_delay[DIR_LEN];
  bool moving[DIR_LEN];
  bool dispatched[DIR_LEN];
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
