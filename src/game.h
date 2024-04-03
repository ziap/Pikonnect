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
  int max_width;
  int input_width;
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
  uint32_t level;
};

struct GameResult {
  uint32_t last_score;
  bool new_best;
};

struct GameMenu {
  GameBoard board;
  Queue search_queue;

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

  uint32_t score;
  uint32_t remaining;
  float score_timer;
  
  int grid_side;
  int x0;
  int y0;
};

enum WonBtns {
  WON_BTN_NEXT,
  WON_BTN_MENU,
  WON_BTN_LEN
};

enum LostBtns {
  LOST_BTN_RESTART,
  LOST_BTN_MENU,
  LOST_BTN_LEN
};

struct WonMenu {
  WonBtns selection;
};

struct LostMenu {
  LostBtns selection;
};

struct LeaderboardMenu {
  User *top_users[5];
  uint32_t top_len;
  int index_width;
};

struct Game {
  // Global state
  UserTable users;
  User *current_user;
  GameConfig config;
  GameResult result;

  union {
    LoginMenu login;
    HomeMenu home;
    LevelsMenu level;
    GameMenu game;
    WonMenu won;
    LostMenu lost;
    LeaderboardMenu leaderboard;
  } menu;
};

extern void Game_init(Game *game);
extern void Game_deinit(Game *game);

#endif
