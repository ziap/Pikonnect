#ifndef GAME_H
#define GAME_H

#include "search.h"
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
  uint32_t current_selection; 
  float selection_lerp[LEVEL_COUNT];
};

struct GameConfig {
  GameMode gamemode;
  int level;
};

struct GameMenu {
  uint32_t start_time;
  GameBoard board;

  Index pos;

  float as_delay[DIR_LEN];
  Dir move_direction;
  bool dispatched[DIR_LEN];

  Index selection;
  bool selecting;
  float selection_lerp;

  Path path;
  int path_val;
  float path_lerp;
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
