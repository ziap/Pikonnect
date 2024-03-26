#ifndef DATA_H
#define DATA_H

#include <stdint.h>

struct User {
  char* name;
  uint64_t password_hash;
};

struct UserTable {
  User *users;
  int len;
};

extern void UserTable_load(UserTable *table);
extern void UserTable_save(UserTable *table);

extern User *UserTable_login(UserTable *table, const char *username, const char *password);

#endif
