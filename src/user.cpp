#include "user.h"
#include "utils.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char data_path[] = "users.db";

void UserTable_load(UserTable *table) {
  FILE *f = fopen(data_path, "rb");

  if (f) {
    fread(&table->names_len, sizeof(table->names_len), 1, f);

    table->names = (char*)malloc(table->names_len + 1);
    fread(table->names, table->names_len, 1, f);
    fread(&table->len, sizeof(table->len), 1, f);
    table->users = (User*)malloc((table->len + 1) * sizeof(User));

    char *ptr = table->names;

    for (uint32_t i = 0; i < table->len; ++i) {
      table->users[i].name = ptr;
      ptr += strlen(ptr) + 1;
      fread(&table->users[i].info, sizeof(table->users[i].info), 1, f);
    }
  } else {
    table->users = (User*)malloc(sizeof(User));
    table->len = 0;
    table->names = nullptr;
    table->names_len = 0;
  }

  table->new_name = nullptr;
}

void UserTable_save(UserTable *table) {
  FILE *f = fopen(data_path, "wb");

  if (f) {
    uint32_t names_len = table->names_len;
    if (table->new_name) names_len += strlen(table->new_name) + 1;
    fwrite(&names_len, sizeof(names_len), 1, f);
    fwrite(table->names, table->names_len, 1, f);
    if (table->new_name) fwrite(table->new_name, names_len - table->names_len, 1, f);
    fwrite(&table->len, sizeof(table->len), 1, f);
    for (uint32_t i = 0; i < table->len; ++i) {
      User *user = table->users + i;
      fwrite(&user->info, sizeof(user->info), 1, f);
    }
  } else {
    fprintf(stderr, "ERROR: Failed to save data to `%s`: %s\n", data_path, strerror(errno));
  }

  if (table->new_name) free(table->new_name);
  free(table->names);
  free(table->users);
}

User *UserTable_login(UserTable *table, const char *username, const char *password) {
  const uint64_t hash = fasthash64(password, strlen(password), PASSWORD_SEED);

  for (uint32_t i = 0; i < table->len; ++i) {
    User *user = table->users + i;
    if (strcmp(username, user->name) == 0) {
      return hash == user->info.password_hash ? user : nullptr;
    }
  }

  User *user = table->users + table->len++;
  int len = strlen(username);
  char *buf = (char*)malloc(len + 2);
  memcpy(buf, username, len + 1);
  user->name = table->new_name;
  user->info.password_hash = hash;

  for (int i = 0; i < LEVEL_COUNT; ++i) {
    user->info.solve_time[i] = NOT_SOLVED;
    buf[len] = i + 1;
    for (int j = 0; j < GAMEMODE_LEN; ++j) {
      buf[len + 1] = j + 1;
      user->info.random_state[i][j] = fasthash64(buf, len + 2, hash);
    }
  }
  
  buf[len] = 0;
  table->new_name = buf;

  return user;
}
