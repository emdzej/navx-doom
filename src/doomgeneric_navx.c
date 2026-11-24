/*
 * doomgeneric on OS-9000 (the BMW MK4 nav), built with navx cc: the DG_*
 * functions doomgeneric calls. Time and sleep are the OS's; frames and
 * keys go to the platform (navx_platform.h).
 */
#include <navx/os9.h>
#include "doomgeneric.h"
#include "navx_platform.h"

void DG_Init(void) {}

void DG_DrawFrame(void) { navx_platform_frame(DG_ScreenBuffer, DOOMGENERIC_RESX, DOOMGENERIC_RESY); }

/* F_TIME: seconds, and the tick rate (high half) with the tick in the second (low half). */
uint32_t DG_GetTicksMs(void) {
  struct { struct os9_pb h; u_int32 time, ticks; } pb = { { F_TIME, 1, 0, sizeof pb }, 0, 0 };
  if (os9_call(&pb)) return 0;
  u_int32 rate = pb.ticks >> 16;
  return pb.time * 1000 + (rate ? (pb.ticks & 0xffff) * 1000 / rate : 0);
}

void DG_SleepMs(uint32_t ms) { os9_sleep(ms >= 10 ? ms / 10 : 1); /* ticks of 10 ms */ }

int DG_GetKey(int *pressed, unsigned char *key) { return navx_platform_key(pressed, key); }

void DG_SetWindowTitle(const char *title) { (void)title; }

int main(int argc, char **argv) {
  navx_platform_init(argc, argv);
  doomgeneric_Create(argc, argv);
  for (;;) doomgeneric_Tick();
}
