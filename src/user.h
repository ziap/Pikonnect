#ifndef USER_H
#define USER_H

#include "config.h"

struct UserInfo {
  uint64_t password_hash;
  uint32_t solve_time[LEVEL_COUNT][GAMEMODE_LEN];
  uint64_t random_state[LEVEL_COUNT][GAMEMODE_LEN];
};

struct User {
  char* name;
  UserInfo info;
};

struct UserTable {
  char *names;
  uint32_t names_len;
  char *new_name;
  User *users;
  uint32_t len;
};

extern void UserTable_load(UserTable *table);
extern void UserTable_save(UserTable *table);

extern User *UserTable_login(UserTable *table, const char *username, const char *password);

#endif
