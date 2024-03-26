#ifndef GAME_H
#define GAME_H

#include "login.h"
#include "home.h"
#include "data.h"

struct Game {
  UserTable users;
  LoginMenu login_menu;
  HomeMenu home_menu;
  User *current_user;
};

extern Game Game_create(void);
extern void Game_deinit(Game *game);

#endif
