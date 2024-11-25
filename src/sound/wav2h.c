#include <raylib.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  InitAudioDevice();

  for (int i = 1; i < argc; ++i) {
    const char *file_name = argv[i];
    int len = strlen(file_name);
    char *out_name = (char*)malloc(len + 3);

    Wave wav = LoadWave(file_name);
    memcpy(out_name, file_name, len);
    memcpy(out_name + len, ".h", 3);
    ExportWaveAsCode(wav, out_name);
    UnloadWave(wav);

    free(out_name);
  }

  return 0;
}
