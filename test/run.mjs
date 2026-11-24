#!/usr/bin/env node
// Run Doom in the navx emulator (HLE) with your WAD and check what it draws.
//
//   NAVX_DOOM_WAD=/path/DOOM1.WAD NAVX_DIR=../navx node test/run.mjs [--shots dir]
//
// No WAD in the repository: bring your own (the shareware DOOM1.WAD works).
// Builds dist/doom first if it's missing.
import { execFileSync } from 'node:child_process';
import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { parseArgs } from 'node:util';
import assert from 'node:assert/strict';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const navx = resolve(process.env.NAVX_DIR ?? join(root, '../navx'));
const wad = process.env.NAVX_DOOM_WAD;
if (!wad) {
  console.log('NAVX_DOOM_WAD not set: skipped');
  process.exit(0);
}
const { values } = parseArgs({ options: { shots: { type: 'string' } } });
const load = (p) => import(pathToFileURL(join(navx, p)).href);
const { Machine, MemoryFileStore } = await load('packages/emu/dist/index.js');
const { Os9Module } = await load('packages/os9/dist/index.js');
const { encodePng } = await load('packages/images/dist/index.js');

const module = join(root, 'dist/doom');
if (!existsSync(module)) execFileSync(process.execPath, [join(root, 'tools/build.mjs')], { stdio: 'inherit' });

const W = 320, H = 200;
/** Boot Doom with `keys` ([pressed, key]…) waiting in its pipe; frame `at` as RGBA. */
function run(at, keys = []) {
  const nvram = new MemoryFileStore();
  nvram.write('doom.wad', new Uint8Array(readFileSync(wad)));
  const m = new Machine({ nvram, stdout: () => {} });
  m.map(new Os9Module(new Uint8Array(readFileSync(module))));
  // Doom reads them once its loop runs; the host's end stays open, so the pipe stays.
  const pipe = m.devices.get('pipe').open({ name: 'doom_keys', full: '/pipe/doom_keys', mode: 2, create: true });
  pipe.write(new Uint8Array(keys.flat()));
  const args = ['-iwad', '/nvr_ramdisk/doom.wad', '-frames', '/nvr_ramdisk/frame', '-every', String(at), '-quit', String(at), '-keys', '/pipe/doom_keys'];
  const status = m.run('doom', args, 4e9);
  assert.equal(status, 0, `doom exited with ${status}`);
  const f = nvram.read('frame');
  assert.ok(f && f.length === W * H * 4, 'a frame was written');
  return toRgba(f);
}
function toRgba(f) {
  const rgba = new Uint8Array(W * H * 4);
  for (let i = 0; i < W * H; i++) (rgba[4 * i] = f[4 * i + 1]), (rgba[4 * i + 1] = f[4 * i + 2]), (rgba[4 * i + 2] = f[4 * i + 3]), (rgba[4 * i + 3] = 255);
  return rgba;
}
const colours = (rgba) => new Set(Array.from({ length: W * H }, (_, i) => (rgba[4 * i] << 16) | (rgba[4 * i + 1] << 8) | rgba[4 * i + 2])).size;
const differ = (a, b) => a.some((v, i) => v !== b[i]);
const shot = (name, rgba) => {
  if (!values.shots) return;
  mkdirSync(values.shots, { recursive: true });
  writeFileSync(join(values.shots, `${name}.png`), encodePng({ width: W, height: H, data: rgba }));
};

const title = run(50);
shot('title', title);
assert.ok(colours(title) > 32, 'the title screen is drawn');
const demo = run(700);
shot('demo', demo);
assert.ok(differ(title, demo), 'the demo plays after the title');
const KEY_ESCAPE = 27;
const menu = run(80, [[1, KEY_ESCAPE], [0, KEY_ESCAPE]]);
shot('menu', menu);
assert.ok(differ(menu, run(80)), 'Escape opens the menu');
console.log('doom: title, demo and menu OK');
