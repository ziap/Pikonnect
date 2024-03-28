#include "user.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char data_path[] = "users.db";

static uint64_t mix(uint64_t h) {
  h ^= h >> 23;
  h *= 0x2127599bf4325c37ULL;
  h ^= h >> 47;
  return h;
}

// Copied from: https://code.google.com/archive/p/fast-hash/
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
  // std::ifstream fin(data_path, std::ios::binary | std::ios::ate);
  FILE *f = fopen(data_path, "rb");

  if (f) {
    table->len = 0;

    // https://stackoverflow.com/a/525103
    // Read entire file into a dynamically allocated array
    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *bytes = (char*)malloc(file_size);
    fread(bytes, file_size, 1, f);
    fclose(f);

    // Allocate an extra user in case of new user creation
    const char *ptr = bytes;
    memcpy(&table->len, ptr, sizeof(table->len));
    table->users = (User*)malloc((table->len + 1) * sizeof(User));
    ptr += sizeof(table->len);

    for (uint32_t i = 0; i < table->len; ++i) {
      User *user = table->users + i;
      int len = strlen(ptr);
      user->name = (char*)malloc(len + 1);
      memcpy(table->users[i].name, ptr, len + 1);
      ptr += len + 1;
      memcpy(&user->password_hash, ptr, sizeof(user->password_hash));
      ptr += sizeof(user->password_hash);
      for (int j = 0; j < LEVEL_COUNT; ++j) {
        memcpy(user->solve_time + j, ptr, sizeof(user->solve_time[j]));
        ptr += sizeof(user->solve_time[j]);
      }
    }
    free(bytes);
  } else {
    table->users = (User*)malloc(sizeof(User));
    table->len = 0;
  }
}

void UserTable_save(UserTable *table) {
  FILE *f = fopen(data_path, "wb");

  if (f) {
    fwrite(&table->len, sizeof(table->len), 1, f);
    for (uint32_t i = 0; i < table->len; ++i) {
      User *user = table->users + i;
      char *ptr = user->name;
      int len = strlen(ptr);
      fwrite(ptr, len + 1, 1, f);
      uint64_t hash = user->password_hash;
      fwrite(&hash, sizeof(hash), 1, f);
      for (int j = 0; j < LEVEL_COUNT; ++j) {
        uint32_t time = user->solve_time[j];
        fwrite(&time, sizeof(time), 1, f);
      }
    }
  } else {
    fprintf(stderr, "ERROR: Failed to save data to `%s`: %s", data_path, strerror(errno));
  }

  for (uint32_t i = 0; i < table->len; ++i) {
    free(table->users[i].name);
  }
  
  free(table->users);
}

User *UserTable_login(UserTable *table, const char *username, const char *password) {
  const uint64_t hash = fasthash64(password, strlen(password), 42);

  for (uint32_t i = 0; i < table->len; ++i) {
    User *user = table->users + i;
    if (strcmp(username, user->name) == 0) {
      return hash == user->password_hash ? user : nullptr;
    }
  }

  User *user = table->users + table->len++;
  int len = strlen(username);
  user->name = (char*)malloc(len + 1);
  memcpy(user->name, username, len + 1);
  user->password_hash = hash;

  for (int i = 0; i < LEVEL_COUNT; ++i) {
    user->solve_time[i] = NOT_SOLVED;
  }

  return user;
}
