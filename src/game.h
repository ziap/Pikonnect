#ifndef GAME_H
#define GAME_H

#include <raylib.h>

#include "search.h"
#include "user.h"
#include "board_array.h"

// This file contains all structures used for representing the game. It can be
// split into several smaller files but this is good enough for now.

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

  int grid_rows;
  int grid_total;
};

const int BG_SQUARE_COUNT = 64;

struct BackgroundSquare {
  float x;
  float y;
  float side;
  int col;

  float fbm_offset_x[5];
  float fbm_offset_y[5];
  float freq_x;
  float freq_y;
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

  bool hinting;
  Index hint_indices[2];
  float hint_timer;

  int collapse_row[2];
  int collapse_row_len;

  int collapse_col[2];
  int collapse_col_len;

  float collapse_timer;

  int grid_side;
  int x0;
  int y0;

  BackgroundSquare background_squares[BG_SQUARE_COUNT];
  float background_timer;
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

enum GameSounds {
  SOUND_CLICK,
  SOUND_SELECT,
  SOUND_CORRECT,
  SOUND_LEN
};

struct Game {
  // These are data that can be accessed in any scene or game object
  UserTable users;
  User *current_user;

  struct {
    GameMode gamemode;
    uint32_t level;
  } config;

  struct {
    uint32_t last_score;
    bool new_best;
  } result;

  Sound sounds[SOUND_LEN];

  // These are data that are specific to a scene
  // A union is used to save memory and to prevent data of a scene from leaking
  // into another scene
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
