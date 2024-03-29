#include "user.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char data_path[] = "users.db";

// Hash the user's password to:
// - Obfuscate it
// - Storing a fixed-length hash is easier
// Copied from: https://code.google.com/archive/p/fast-hash/
static uint64_t mix(uint64_t h) {
  h ^= h >> 23;
  h *= 0x2127599bf4325c37ULL;
  h ^= h >> 47;
  return h;
}

static uint64_t fasthash64(const void *buf, size_t len, uint64_t seed) {
	const uint64_t    m = 0x880355f21e6d1965ULL;
	const uint64_t *pos = (const uint64_t *)buf;
	const uint64_t *end = pos + (len / 8);
	const uint8_t *pos2;
	uint64_t h = seed ^ (len * m);
	uint64_t v;

	while (pos != end) {
		v  = *pos++;
		h ^= mix(v);
		h *= m;
	}

	pos2 = (const uint8_t*)pos;
	v = 0;

	switch (len & 7) {
	case 7: v ^= (uint64_t)pos2[6] << 48;
	case 6: v ^= (uint64_t)pos2[5] << 40;
	case 5: v ^= (uint64_t)pos2[4] << 32;
	case 4: v ^= (uint64_t)pos2[3] << 24;
	case 3: v ^= (uint64_t)pos2[2] << 16;
	case 2: v ^= (uint64_t)pos2[1] << 8;
	case 1: v ^= (uint64_t)pos2[0];
		h ^= mix(v);
		h *= m;
	}

	return mix(h);
} 

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
  const uint64_t hash = fasthash64(password, strlen(password), 42);

  for (uint32_t i = 0; i < table->len; ++i) {
    User *user = table->users + i;
    if (strcmp(username, user->name) == 0) {
      return hash == user->info.password_hash ? user : nullptr;
    }
  }

  User *user = table->users + table->len++;
  int len = strlen(username);
  table->new_name = (char*)malloc(len + 1);
  memcpy(table->new_name, username, len + 1);
  user->name = table->new_name;
  user->info.password_hash = hash;

  for (int i = 0; i < LEVEL_COUNT; ++i) {
    user->info.solve_time[i] = NOT_SOLVED;
  }

  return user;
}
