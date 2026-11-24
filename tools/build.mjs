#!/usr/bin/env node
// Build Doom as an OS-9000 program module with the navx SDK.
//
//   NAVX_DIR=../navx node tools/build.mjs [-o dist/doom] [--backend src/platform_emu.c]
//
// NAVX_DIR is a built navx checkout (pnpm build); default ../navx.
// --backend picks the platform (navx_platform.h); default the emulator's.
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { parseArgs } from 'node:util';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const navx = resolve(process.env.NAVX_DIR ?? join(root, '../navx'));
const { values } = parseArgs({ options: { out: { type: 'string', short: 'o' }, backend: { type: 'string' } } });
const { build } = await import(pathToFileURL(join(navx, 'packages/sdk/dist/index.js')).href);

/** doomgeneric's portable sources: its Makefile's list without the platform files. */
export function doomSources() {
  const mk = readFileSync(join(root, 'doomgeneric/Makefile'), 'utf8').match(/^SRC_DOOM = (.*)$/m)[1];
  return mk
    .split(/\s+/)
    .filter((f) => f && !/^doomgeneric_/.test(f) && f !== 'w_file_stdc.o') // src/w_file_navx.c instead
    .map((f) => join(root, 'doomgeneric', f.replace(/\.o$/, '.c')));
}

const out = values.out ?? join(root, 'dist/doom');
const backend = resolve(values.backend ?? join(root, 'src/platform_emu.c'));
const r = build([...doomSources(), join(root, 'src/doomgeneric_navx.c'), join(root, 'src/w_file_navx.c'), backend], {
  name: 'doom',
  largeData: true, // ~420 KB of globals
  padHiLo: true, // Doom multiplies and divides
  stack: 0x10000,
  cflags: ['-Os', '-w', '-I', join(root, 'doomgeneric'), '-I', join(root, 'src'), '-DDOOMGENERIC_RESX=320', '-DDOOMGENERIC_RESY=200'],
});
mkdirSync(dirname(out), { recursive: true });
writeFileSync(out, r.module.bytes);
if (r.module.bytes.length % 4) throw new Error(`${out}: ${r.module.bytes.length} bytes, not a multiple of 4`);
console.log(`doom: ${r.module.bytes.length} bytes (code ${r.codeSize}, data ${r.dataSize}) → ${out}`);
