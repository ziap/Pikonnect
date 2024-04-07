#include "game.h"

#include "sound/click.wav.h"
#include "sound/select.wav.h"
#include "sound/correct.wav.h"

static const Wave sound_wavs[SOUND_LEN] = {
  {
    CLICK_FRAME_COUNT,
    CLICK_SAMPLE_RATE,
    CLICK_SAMPLE_SIZE,
    CLICK_CHANNELS,
    CLICK_DATA
  },
  {
    SELECT_FRAME_COUNT,
    SELECT_SAMPLE_RATE,
    SELECT_SAMPLE_SIZE,
    SELECT_CHANNELS,
    SELECT_DATA
  },
  {
    CORRECT_FRAME_COUNT,
    CORRECT_SAMPLE_RATE,
    CORRECT_SAMPLE_SIZE,
    CORRECT_CHANNELS,
    CORRECT_DATA
  }
};

void Game_init(Game *game) {
  game->current_user = nullptr;
  for (int i = 0; i < SOUND_LEN; ++i) {
    game->sounds[i] = LoadSoundFromWave(sound_wavs[i]);
  }
  UserTable_load(&game->users);
}

void Game_deinit(Game *game) {
  for (int i = 0; i < SOUND_LEN; ++i) {
    UnloadSound(game->sounds[i]);
  }
  UserTable_save(&game->users);
}
