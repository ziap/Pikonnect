#ifndef GAME_H
#define GAME_H

#include "menu_login.h"
#include "menu_home.h"
#include "menu_levels.h"
#include "user.h"

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

extern Game Game_create(void);
extern void Game_deinit(Game *game);

#endif
