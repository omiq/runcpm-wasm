# runcpm-wasm

CP/M 2.2 running in a browser tab, compiled from [RunCPM](https://github.com/MockbaTheBorg/RunCPM) to WebAssembly. No DOSBox, no nested emulators, no disk images to fiddle with. It boots to an `A0>` prompt in well under a second and you type at it like a real machine.

There is a playable copy of Rogue in here too (see below), which is the demo I keep coming back to. If a roguelike works then the interesting parts all work: the Z80 core, the console, and the awkward blocking-input problem that stops most "CP/M in a browser" attempts dead.

## Why this and not DOSBox

The usual way to get CP/M in a browser is to run a wasm build of DOSBox and then run a DOS program inside it that itself emulates CP/M. That is three layers deep, it is big, and feeding your own programs in is a chore.

RunCPM takes a different route. It is a single portable C program that emulates the Z80 (or 8080) directly and reimplements the CP/M BDOS and BIOS in C rather than running Digital Research's original binaries. It ships its own command processor in `ccp.h`. Compile that to wasm and you get CP/M in one flat binary at close to native speed, with no DOS underneath.

Loading programs is easy because RunCPM uses host files as CP/M disks. A drive is just a folder. Drive A user 0 is the directory `A/0`, and `A/0/PROG.COM` is a file you can drop in. In the browser that folder lives in Emscripten's in-memory filesystem, so the harness just writes your `.COM` there before boot.

## The blocking-input problem

This is the part that took the thinking. CP/M console input blocks. Down inside the BDOS, reading a key calls `_getch()`, which sits and waits until a key arrives. You cannot do that on a browser's main thread. Block it and the event loop stops, which means the very keypress you are waiting for can never be delivered, and the tab freezes.

The fix is Emscripten's ASYNCIFY. It rewrites the compiled code so a blocking call can suspend the whole C call stack, hand control back to the browser, and resume later exactly where it left off. In `_getch` it looks like this:

```c
uint8 _getch(void) {
    int c;
    while ((c = cpm_js_getch()) < 0) {  // ask JS for a queued key, -1 if none
        emscripten_sleep(8);            // yield to the browser, resume in 8ms
    }
    return (uint8)c;
}
```

`emscripten_sleep` is the yield point. While it sleeps the browser runs, your keydown handler pushes a byte into a JS queue, and on resume `cpm_js_getch()` hands it back. As far as the C code is concerned it just blocked and got a key, which is exactly what CP/M expects.

There is a second, related trap. A program that spins polling `_kbhit()` without ever calling `_getch` (real-time-ish games do this), or one that runs a long stretch with no console call at all (generating a level, say), never gives the event loop a turn either. So there is a small unconditional yield in the CPU loop every so many instructions as a backstop. Both edits live in `src/emscripten.patch`.

## How the port is put together

RunCPM is written so that only one file changes per platform: an `abstraction_*.h` that provides file access and console I/O. There is a POSIX one, a Windows one, an Arduino one. This adds an Emscripten one.

I started from the POSIX abstraction because Emscripten's libc is POSIX-shaped: its in-memory filesystem answers `fopen`, `opendir`, `glob` and friends, so the whole file half compiled unchanged. Only the console half needed replacing, because POSIX consoles use termios raw mode and `poll(stdin)`, none of which mean anything in a browser.

The console half now calls four small bridge functions written in JavaScript through Emscripten's `EM_JS`:

```c
EM_JS(void, cpm_js_putch,  (int ch), { globalThis.CPM.putch(ch); });
EM_JS(int,  cpm_js_getch,  (void),   { return globalThis.CPM.getch(); });
EM_JS(int,  cpm_js_kbhit,  (void),   { return globalThis.CPM.kbhit(); });
EM_JS(void, cpm_js_clrscr, (void),   { globalThis.CPM.clrscr(); });
```

The page provides `globalThis.CPM` with those four methods and owns the terminal. That is the whole contract between the C and the browser.

So the port is three things: the abstraction file (`src/abstraction_emscripten.h`), a two-hunk patch to upstream `main.c` and `cpu1.h` (`src/emscripten.patch`), and a build script. Nothing else in RunCPM is touched, which is exactly what its design is meant to allow.

## Build it yourself

You need the Emscripten SDK (emsdk) installed. The build looks for it at `~/emsdk` by default; set `EMSDK` if yours lives elsewhere.

```bash
git clone --recursive https://github.com/omiq/runcpm-wasm.git
cd runcpm-wasm
./build.sh                       # copies the abstraction in, applies the patch, runs emcc
cd web && python3 -m http.server 8799
# open http://localhost:8799/index.html, click the screen, type DIR
```

`build.sh` is safe to re-run; it notices when the patch is already applied. The build output (`web/runcpm.js` and `web/runcpm.wasm`, about 205 KB together) is committed, so if you only want to run it you can skip emsdk entirely and go straight to the http.server step.

## What's in web/

`index.html` is a minimal glass-teletype: a fixed 80 by 25 grid that interprets the CP/M console byte stream (line feed, carriage return, backspace, form feed, printable bytes). It is enough for line-oriented programs, the command processor, DIR, BASIC and so on. It does not interpret cursor-positioning escape sequences, so full-screen apps will not lay out correctly in it.

`rogue.html` is the same idea with a small VT100 terminal bolted on: cursor addressing, erase-to-end-of-line and erase-display, relative cursor moves, and it swallows the colour and DEC-private sequences it does not need. That is enough to make a full-screen curses program readable. It preloads `ROGUE.COM` into drive A and boots to the prompt so you type `ROGUE` yourself.

Both pages preload a small default disk from `web/disk/` onto drive A user 0, so `DIR` shows something the moment it boots:

- `README.TXT` a short guide, shown with `TYPE README.TXT`.
- `HELLO.COM` hello world in C, compiled with z88dk's `+cpm` target.
- `GREET.COM` the same thing in Z80 assembly, about sixty bytes.
- `ROGUE.COM` the Rogue port described below.

The sources for the two demos, and a script that rebuilds them, are in `cpm-programs/demos`. I have deliberately not bundled the classic Digital Research utilities (PIP, STAT, ED and so on); they are still under copyright, so add your own copy if you want them.

To run your own program, copy the pattern in `rogue.html`: write your `.COM` into `/A/0` with `Module.FS.writeFile` before calling `Module.callMain`.

## About the Rogue binary

The `cpm-programs/rogue` folder has the demo program and its docs. The binary identifies itself as `ROGUE - V1.7   DPG 1985` and was built for a Televideo TS803 terminal, so it is a 1985 CP/M port of Rogue with the porter's initials in the version string. `rogue.sub` is the original CP/M SUBMIT script that patches the base `rogue.com` (through `DDT` and a hex patch) into the VT100 build `rogue-vt.com`, which is the one the web harness uses. I have left that recipe in because the history is half the fun.

Rogue itself was written by Michael Toy, Ken Arnold and Glenn Wichman. This CP/M port is included as a period demonstration. If you hold the rights to it and would rather it were not here, open an issue and I will take it down.

## Status

This is a proven spike, not a finished product. What works: the CPU, the reimplemented BDOS and BIOS, the internal command processor, two-way console, the ASYNCIFY input trick, the in-memory disk, and Rogue playing through to a fresh dungeon each run. That was the risky part, and it holds up.

Rough edges and things I would still like to do:

- The glass-TTY in `index.html` does not handle cursor positioning. Full-screen apps outside the VT100 harness will need a proper terminal (something like xterm.js, or fuller ADM-3A/VT100 handling).
- No auto-run yet. Both harnesses boot to the prompt on purpose. For Rogue that is almost a feature: the human-timed wait at the prompt varies the machine state, which the game seeds its RNG from, so you get a different dungeon each run. A fixed auto-start would hand you the same dungeon every time.
- A path for compiling your own C to a `.COM` (via z88dk's `+cpm` target) and running it here would close the loop.

## Licence

The port code in this repository (the Emscripten abstraction, the patch, the build script and the web harnesses) is MIT; see `LICENSE`. RunCPM is MIT and stays under its own licence in the submodule (`RunCPM/LICENSE`). Because it is built with `-DCCP_INTERNAL` the compiled binary contains only RunCPM's own code, so no Digital Research CP/M code is distributed.

Credit where it is due: RunCPM is by MockbaTheBorg. This repository only teaches it to run in a browser.
