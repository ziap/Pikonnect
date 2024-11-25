#ifndef USER_H
#define USER_H

#include <stdbool.h>
#include "config.h"

typedef struct {
  uint64_t password_hash;
  uint64_t random_state;
  bool passed_tutorial;
  uint32_t best_score;
  uint32_t unlocked[GAMEMODE_LEN];
} UserInfo;

// The user is stored in a key-value pair, with the key being a dynamically
// sized string and the value being statically sized variables
typedef struct {
  char* name;
  UserInfo info;
} User;

typedef struct {
  // A region of null-separated names, this is done so that we don't have to
  // manage memory for individual names.
  char *names;
  uint32_t names_len;

  // The name of the newly-created user. Currently the login scene is only shown
  // once so the maximum number of new users is one.
  char *new_name;
  User *data;
  uint32_t len;
} UserTable;

extern void UserTable_load(UserTable *table);
extern void UserTable_save(UserTable *table);

extern User *UserTable_login(UserTable *table, const char *username, const char *password);

#endif
