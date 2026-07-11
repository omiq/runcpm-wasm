# About RunCPM-WASM

CP/M 2.2 running in a browser tab, converted from the original [RunCPM](https://github.com/MockbaTheBorg/RunCPM) to WebAssembly. 

<img width="708" height="462" alt="Rogue.COM" src="https://github.com/user-attachments/assets/66cc91c0-92b6-4524-b51e-06bbc3450e5c" />


It boots to an `A0>` prompt from which you can operate it like a real CP/M machine but in your web browser.

There is a playable copy of Rogue in here too (see below), which is the demo I keep coming back to because if a roguelike works then the crucial parts are confirmed to be working

* Z80
* vt100 console
* disks/files
* bios etc

## How it works and why not my old DOSBox approach?

My last attempt at CP/M in a browser ran a wasm build of DOSBox and then in turn launched a DOS CP/M emulation program inside it. 

It worked but at three layers deep, it is big and messy, and feeding your own programs into it and run them is a chore.

RunCPM is a single portable C program that emulates the Z80 (or 8080) directly and reimplements the CP/M BDOS and BIOS in C rather than running Digital Research's original binaries. It ships its own command processor in `ccp.h`. Compile that to wasm and you get CP/M in one package with nothing else required.

Loading programs is easier because RunCPM uses presents the host's files as CP/M disks. A drive is just a named folder. Drive A user 0 is the directory `A/0`, and `A/0/PROG.COM` is a file you can drop right in. 

For the browser version, that folder lives in Emscripten's virtual filesystem which is accessible from JS, so we just put your `.COM` there before mounting.

## The blocking-input problem

A CP/M console input blocks inside the BDOS, reading a key calls `_getch()`, which waits until a key is pressed. 

You cant do that in a browser's main thread. The event loop stops, which means the keypress you're waiting for can never arrive, and the browser freezes.

The fix is to use Emscripten's `ASYNCIFY`. It rewrites the code so a blocking call can hand control back to the browser, and resume later exactly where it left off. In `_getch` it looks like this:

```c
uint8 _getch(void) {
    int c;
    while ((c = cpm_js_getch()) < 0) {  // ask JS for a queued key, -1 if none
        emscripten_sleep(8);            // yield to the browser, resume in 8ms
    }
    return (uint8)c;
}
```

`emscripten_sleep` is the yield, while it sleeps the browser runs as usual, your key down handler pushes a byte into a JS queue, and on resume `cpm_js_getch()` returns it back. As far as the C code is concerned it just blocked and then got a key, which is exactly what CP/M looks for.

There is a second, related problem in that a program that spins polling `_kbhit()` without ever calling `_getch` (games do this), or one that runs a long time with no console call at all (generating a level, say), never gives the event loop a turn either. So there is a small unconditional yield in the CPU loop every N instructions as a plan B. That yield, and a couple of other browser-only tweaks, are in `src/emscripten.patch` (described below).

## How the port is put together

RunCPM is written so that only one file changes per platform: an `abstraction_*.h` that provides file access and console I/O. There is a POSIX one, a Windows one, an Arduino one. This adds an Emscripten one.

I started from the POSIX abstraction because Emscripten's libc is POSIX-ish: its in-memory filesystem answers `fopen`, `opendir`, `glob` and friends, so the whole file half compiled unchanged. Only the console half needed replacing, because POSIX consoles use termios raw mode and `poll(stdin)`, none of which mean anything to a web browser.

The console half now calls four small functions written in JavaScript through Emscripten's `EM_JS`:

```c
EM_JS(void, cpm_js_putch,  (int ch), { globalThis.CPM.putch(ch); });
EM_JS(int,  cpm_js_getch,  (void),   { return globalThis.CPM.getch(); });
EM_JS(int,  cpm_js_kbhit,  (void),   { return globalThis.CPM.kbhit(); });
EM_JS(void, cpm_js_clrscr, (void),   { globalThis.CPM.clrscr(); });
```

The page provides `globalThis.CPM` with those four methods and runs the terminal. That is the plumbing between our C code and the browser.

So the port is three things: the abstraction file (`src/abstraction_emscripten.h`), a small patch to upstream `main.c` and `cpu1.h` (`src/emscripten.patch`), and a build script. The patch is all `#ifdef __EMSCRIPTEN__` so it selects the abstraction, adds the CPU-loop yield described above, skips RunCPM's boot-time speed benchmark (a ~290M-instruction loop that makes the browser look frozen), and prints the version message and stuff. Nothing else in RunCPM is changed.

## Run it or build it yourself

You need the Emscripten SDK (emsdk) installed. The build looks for it at `~/emsdk` by default; set `EMSDK` if yours lives elsewhere.

```bash
git clone --recursive https://github.com/omiq/runcpm-wasm.git
cd runcpm-wasm
./build.sh  # copies the abstraction in, applies the patch, runs emcc
cd web && python3 -m http.server 8799
# open http://localhost:8799/index.html, click the screen, type DIR
```

`build.sh` is safe to re-run. Build output (`web/runcpm.js` and `web/runcpm.wasm`, about 205 KB together) is available, so if you only want to run it you can skip emsdk entirely and go straight to the http.server step.

## What's in web/

`index.html` is the main page: an 80 by 24 terminal with a small VT100 compatibility layer such as cursor addressing, erase-to-end-of-line and erase-display, relative cursor moves, and `ESC[?25l`/`ESC[?25h` to hide or show the cursor (it ignores the colour and other DEC-private sequences it does not understand). That is enough for most line-oriented programs (the command processor, DIR, BASIC) and full-screen curses programs. It has a blinking block cursor, and nothing is clipped when the screen scrolls. It boots to the prompt with the default disk loaded, so `DIR` shows something straight away.

`rogue.html` is the same terminal wired to auto-run: it just preloads `ROGUE.COM` and launches it for you.

Both pages preload a small default disk from `web/disk/` onto drive A user 0, so `DIR` shows something the moment it boots:

- `README.TXT` a short guide, shown with `TYPE README.TXT`.
- `HELLO.COM` hello world in C, compiled with z88dk's `+cpm` target.
- `GREET.COM` the same thing in Z80 assembly, about sixty bytes.
- `ROGUE.COM` the Rogue port described below.

The sources for the two demos, and a script that rebuilds them, are in `cpm-programs/demos`. I have deliberately not bundled the classic Digital Research utilities (PIP, STAT, ED and so on) because copyright, so add your own copy if you want them.

For a fuller setup, `index.html` will also read an optional `web/disk/manifest.json` describing several drives (`{ "drives": { "A": [...], "B": [...], "C": [...] } }`) and preload each from `web/disk/<DRIVE>/`. With no manifest present (as in this repo) it just loads the small default disk above onto drive A. When the manifest lists a lot of files the fetches run in parallel so boot stays fast.

To run your own program, copy `rogue.html`: write your `.COM` into `/A/0` with `Module.FS.writeFile`, then write a one-line `AUTOEXEC.TXT` naming it to the MEMFS root (`Module.FS.writeFile('/AUTOEXEC.TXT', ...)`), both before calling `Module.callMain`. The build sets `BOOTONLY=TRUE`, so that command runs once at cold boot. Drop the `AUTOEXEC.TXT` line to boot to the `A0>` prompt instead.

## About the Rogue binary

The `cpm-programs/rogue` folder has the demo program and its docs. The binary identifies itself as `ROGUE - V1.7   DPG 1985` and was built for a Televideo TS803 terminal, so it is a 1985 CP/M port of Rogue with the porter's initials in the version string. `rogue.sub` is the original CP/M SUBMIT script that patches the base `rogue.com` (through `DDT` and a hex patch) into the VT100 build `rogue-vt.com`, which is the one the web load uses. I have left that in because the history is half the fun.

Rogue itself was written by Michael Toy, Ken Arnold and Glenn Wichman. This CP/M port is included as a period demonstration. If you hold the rights to it and would rather it were not here, open an issue and I will take it down.

## Status

The core is up and running live at [cpm.retrogamecoders.com](https://cpm.retrogamecoders.com). What works: the CPU, the reimplemented BDOS and BIOS, the internal command processor, the two-way VT100 console, the ASYNCIFY input trick, the in-memory disk (including several drives), and real 1980s CP/M software (Microsoft BASIC, WordStar, the Hi-Tech C compiler, Turbo Pascal, Rogue).

The compile-your-own-`.COM` loop is up too: the [Retro Game Coders IDE](https://ide.retrogamecoders.com/?platform=cpm) has a CP/M platform that compiles your C and runs the result in the emulator.

## Rough edges and things I would still like to do:

- The VT100 terminal is minimal, enough for the CCP and for curses programs like Rogue. Exotic escape sequences are ignored rather than interpreted. Something like dropping in a full terminal emulator (xterm.js) would be an upgrade.
- `tools/extract-myz80.sh` pulls loose files out of MYZ80 `.DSK` images (the disk format the old DOSBox site used), which is how the bigger multi-drive sets were built. MYZ80 uses a non-standard layout, so it needs the custom cpmtools geometry the script documents.

## Licence

The port code in this repository (the Emscripten abstraction, the patch, the build script and the web harnesses) is MIT; see `LICENSE`. RunCPM is MIT and stays under its own licence in the submodule (`RunCPM/LICENSE`). Because it is built with `-DCCP_INTERNAL` the compiled binary contains only RunCPM's own code, so no Digital Research CP/M code is distributed.

Credit where it is due: RunCPM is by MockbaTheBorg. This repository only teaches it to run in a browser so I could have CP/M in my online IDE.
