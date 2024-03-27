#ifndef DATA_H
#define DATA_H

#include "config.h"

struct User {
  char* name;
  uint64_t password_hash;
  uint32_t solve_time[LEVEL_COUNT];
};

struct UserTable {
  User *users;
  uint32_t len;
};

extern void UserTable_load(UserTable *table);
extern void UserTable_save(UserTable *table);

extern User *UserTable_login(UserTable *table, const char *username, const char *password);

#endif
