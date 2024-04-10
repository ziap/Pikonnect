#ifndef USER_H
#define USER_H

#include "config.h"

struct UserInfo {
  uint64_t password_hash;
  uint64_t random_state;
  bool passed_tutorial = false;
  uint32_t best_score;
  uint32_t unlocked[GAMEMODE_LEN];
};

// The user is stored in a key-value pair, with the key being a dynamically
// sized string and the value being statically sized variables
struct User {
  char* name;
  UserInfo info;
};

struct UserTable {
  // A region of null-separated names, this is done so that we don't have to
  // manage memory for individual names.
  char *names;
  uint32_t names_len;

  // The name of the newly-created user. Currently the login scene is only shown
  // once so the maximum number of new users is one.
  char *new_name;
  User *data;
  uint32_t len;
};

extern void UserTable_load(UserTable *table);
extern void UserTable_save(UserTable *table);

extern User *UserTable_login(UserTable *table, const char *username, const char *password);

#endif
