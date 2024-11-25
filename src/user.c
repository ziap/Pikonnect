#include "user.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

static const char data_path[] = "users.db";

// Load data from the file
void UserTable_load(UserTable *table) {
  FILE *f = fopen(data_path, "rb");

  if (f) {
    // Read the length of the table
    fread(&table->names_len, sizeof(table->names_len), 1, f);

    // Read the dynamically sized string
    table->names = (char*)malloc(table->names_len + 1);
    fread(table->names, table->names_len, 1, f);
    fread(&table->len, sizeof(table->len), 1, f);
    table->data = (User*)malloc((table->len + 1) * sizeof(User));

    char *ptr = table->names;

    // Assign the name and read the users' data
    for (uint32_t i = 0; i < table->len; ++i) {
      table->data[i].name = ptr;
      ptr += strlen(ptr) + 1;
      fread(&table->data[i].info, sizeof(table->data[i].info), 1, f);
    }
  } else {
    // File doesn't exist, create a new user table
    table->data = (User*)malloc(sizeof(User));
    table->len = 0;
    table->names = NULL;
    table->names_len = 0;
  }

  table->new_name = NULL;
}

// Write data to a file
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
      User *user = table->data + i;
      fwrite(&user->info, sizeof(user->info), 1, f);
    }
  } else {
    fprintf(stderr, "ERROR: Failed to save data to `%s`: %s\n", data_path, strerror(errno));
  }

  // This is called when the program closes, so we also deallocate memories
  if (table->new_name) free(table->new_name);
  free(table->names);
  free(table->data);
}

User *UserTable_login(UserTable *table, const char *username, const char *password) {
  const uint64_t hash = fasthash64(password, strlen(password), MAGIC);

  for (uint32_t i = 0; i < table->len; ++i) {
    User *user = table->data + i;

    if (strcmp(username, user->name) == 0) {
      return hash == user->info.password_hash ? user : NULL;
    }
  }

  User *user = table->data + table->len++;
  const int len = strlen(username);
  char *buf = (char*)malloc(len + 1);

  memcpy(buf, username, len + 1);
  table->new_name = buf;

  user->name = table->new_name;
  user->info.password_hash = hash;
  user->info.passed_tutorial = false;
  user->info.best_score = 0;
  memset(user->info.unlocked, 0, sizeof(user->info.unlocked));

  user->info.random_state = fasthash64(username, len, hash) + table->len;
  pcg32(&user->info.random_state);

  return user;
}
