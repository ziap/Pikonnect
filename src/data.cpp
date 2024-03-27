#include "data.h"

#include <cstdint>
#include <string.h>
#include <fstream>

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
  std::ifstream fin(data_path, std::ios::binary | std::ios::ate);

  if (fin.is_open()) {
    table->len = 0;

    // https://stackoverflow.com/a/525103
    // Read entire file into a dynamically allocated array
    int file_size = fin.tellg();
    fin.seekg(0, std::ios::beg);

    char *bytes = new char[file_size];
    fin.read(bytes, file_size);

    // Allocate an extra user in case of new user creation
    const char *ptr = bytes;
    memcpy(&table->len, ptr, sizeof(table->len));
    table->users = new User[table->len + 1];
    ptr += sizeof(table->len);

    for (uint32_t i = 0; i < table->len; ++i) {
      User *user = table->users + i;
      int len = strlen(ptr);
      user->name = new char[len + 1];
      memcpy(table->users[i].name, ptr, len + 1);
      ptr += len + 1;
      memcpy(&user->password_hash, ptr, sizeof(user->password_hash));
      ptr += sizeof(user->password_hash);
      for (int j = 0; j < LEVEL_COUNT; ++j) {
        memcpy(user->solve_time + j, ptr, sizeof(user->solve_time[j]));
        ptr += sizeof(user->solve_time[j]);
      }
    }

    delete[] bytes;
  } else {
    table->users = new User[1];
    table->len = 0;
  }
}

void UserTable_save(UserTable *table) {
  std::ofstream fout(data_path, std::ios::binary | std::ios::ate);
  fout.write((char*)&table->len, sizeof(table->len));
  for (uint32_t i = 0; i < table->len; ++i) {
    User *user = table->users + i;
    char *ptr = user->name;
    int len = strlen(ptr);
    fout.write(ptr, len + 1);
    uint64_t hash = user->password_hash;
    fout.write((char*)&hash, sizeof(hash));
    for (int j = 0; j < LEVEL_COUNT; ++j) {
      uint32_t time = user->solve_time[j];
      fout.write((const char*)&time, sizeof(time));
    }
    delete[] ptr;
  }

  delete[] table->users;
}

User *UserTable_login(UserTable *table, const char *username, const char *password) {
  uint64_t hash = fasthash64(password, strlen(password), 42);

  for (uint32_t i = 0; i < table->len; ++i) {
    User *user = table->users + i;
    if (strcmp(username, user->name) == 0) {
      return hash == user->password_hash ? user : nullptr;
    }
  }

  User *user = table->users + table->len++;
  int len = strlen(username);
  user->name = new char[len + 1];
  memcpy(user->name, username, len + 1);
  user->password_hash = hash;

  for (int i = 0; i < LEVEL_COUNT; ++i) {
    user->solve_time[i] = NOT_SOLVED;
  }

  return user;
}
