/*
 * What the port needs from where it runs: a place for the frames and a
 * source of key events. platform_emu.c is the one in this repository (the
 * navx emulator: frames into a file, keys from a pipe); another backend
 * can link instead of it.
 */
#ifndef NAVX_PLATFORM_H
#define NAVX_PLATFORM_H
#include <stdint.h>

/* Once, before the game starts, with the program's arguments. */
void navx_platform_init(int argc, char **argv);
/* A finished frame: width × height pixels, 0x00RRGGBB, rows top to bottom. */
void navx_platform_frame(const uint32_t *pixels, int width, int height);
/* The next key event: 1 and *pressed (1 down, 0 up), *key (doomkeys.h); 0 if none waits. */
int navx_platform_key(int *pressed, unsigned char *key);

#endif
