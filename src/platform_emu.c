/*
 * The emulator backend (navx_platform.h). Options, besides Doom's own:
 *
 *   -frames <path>   write every frame to <path> (overwritten each time):
 *                    width × height × 4 bytes, 0x00RRGGBB big-endian
 *   -every <n>       only every n-th frame (default 1)
 *   -quit <n>        exit after frame n
 *   -keys <path>     read key events from <path> (a pipe): two bytes each,
 *                    pressed (1/0) and the doomkeys.h code
 *
 * The host reads the frame file and feeds the pipe; see test/run.mjs.
 */
#include <navx/os9.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "navx_platform.h"

static const char *frames;
static int every = 1, quit, count;
static int keys = -1;

static const char *arg(int argc, char **argv, const char *name) {
  for (int i = 1; i + 1 < argc; i++)
    if (!strcmp(argv[i], name)) return argv[i + 1];
  return NULL;
}

void navx_platform_init(int argc, char **argv) {
  const char *v;
  frames = arg(argc, argv, "-frames");
  if ((v = arg(argc, argv, "-every"))) every = atoi(v) > 0 ? atoi(v) : 1;
  if ((v = arg(argc, argv, "-quit"))) quit = atoi(v);
  if ((v = arg(argc, argv, "-keys"))) keys = os9_open(v, FAM_READ);
}

void navx_platform_frame(const uint32_t *pixels, int width, int height) {
  count++;
  if (frames && count % every == 0) {
    FILE *f = fopen(frames, "wb");
    if (f) fwrite(pixels, 4, (size_t)(width * height), f), fclose(f);
  }
  if (quit && count >= quit) exit(0);
}

int navx_platform_key(int *pressed, unsigned char *key) {
  unsigned char ev[2];
  if (keys < 0 || os9_ready(keys) < 2 || os9_read(keys, ev, 2) != 2) return 0;
  *pressed = ev[0] != 0;
  *key = ev[1];
  return 1;
}
