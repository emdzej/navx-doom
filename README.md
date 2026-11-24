# navx-doom

Doom for the BMW MK4 navigation computer (OS-9000 on MIPS), built with
the [navx](https://github.com/emdzej/navx) SDK. A port of
[doomgeneric](https://github.com/ozkl/doomgeneric). It runs in the navx
emulator; the unit itself is next (a spare one first).

No WAD here: bring your own (the shareware `DOOM1.WAD` works).

## Build and run

```sh
# a built navx checkout next to this one (or NAVX_DIR=…), with LLVM's clang (brew install llvm)
node tools/build.mjs                       # → dist/doom, an OS-9000 program module
NAVX_DOOM_WAD=/path/DOOM1.WAD node test/run.mjs --shots shots
```

`test/run.mjs` boots Doom in the emulator, checks the title screen, the
demo after it and the menu (Escape through the key pipe), and with
`--shots` writes them as PNGs.

## How it fits

- `doomgeneric/`: doomgeneric's sources as they are (upstream
  e257a09527062bfb986b6e5e0780839b6cbd5b71). Its own platform files
  (`doomgeneric_*.c`) aren't built.
- `src/doomgeneric_navx.c`: the `DG_*` functions doomgeneric needs, on
  OS-9000 time and sleep (navx runtime).
- `src/navx_platform.h`: what the port needs from where it runs: a place
  for frames and a source of key events.
- `src/platform_emu.c`: the emulator's platform. Frames go into a file
  (`-frames <path>`, `-every <n>`, `-quit <n>`), keys come from a pipe
  (`-keys <path>`, two bytes per event: pressed, doomkeys.h code). Any other
  backend links in its place (`tools/build.mjs --backend …`).

The SDK features it uses: `--large-data` (about 420 KB of globals, through
a GOT), `--pad-hilo` (Doom multiplies and divides; MIPS I's hi/lo hazard),
and the SDK's C library and soft-float. Doom is fixed-point, so the
soft-float hardly runs.

## Licence

GPL-2.0, as doomgeneric and the Doom source release: see LICENSE.
