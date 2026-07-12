# myconio.h - ANSI console for CP/M C

A tiny header that replaces z88dk's `<conio.h>` with plain ANSI/VT100 escape
sequences, so a single `.COM` renders identically on:

- the RunCPM browser terminal (xterm.js) at cpm.retrogamecoders.com
- a real RC2014 (or any CP/M box) driving a VT100/ANSI terminal over serial
- the 8bitworkshop `cpm` IDE platform

## Why not just use `<conio.h>`?

z88dk's conio is hard-wired to the **ADM-3A**, a 1976 Lear-Siegler terminal
(what Kaypro-era CP/M shipped). It has no colour, and its cursor codes are not
ANSI. You can see this yourself:

```sh
# what z88dk's own conio emits for gotoxy(10,5):
zcc +cpm --generic-console foo.c      # -> ESC = <row+32> <col+32>   (ADM-3A)

# what myconio.h emits for the same call:
#                                       -> ESC [ 5 ; 10 H            (ANSI)
```

The ADM-3A `ESC=` sequence means nothing to xterm.js or a modern terminal, and
z88dk's ANSI console path is broken in the classic library (`fputc_cons_ansi`
is undefined; `-pragma-need=ansiterminal` fails to link). Since we control the
terminal on RunCPM anyway, it's simpler and more portable to emit ANSI directly
than to fight the toolchain. See `../../tools/conio-probe/` for the byte-level
proof of each dialect.

## Build

Needs z88dk on PATH (or set `ZCC`):

```sh
ZCC=~/z88dk/bin/zcc ./build.sh
```

Emits `COLORS.COM`, `DASH.COM`, `MENU.COM` and stages them onto `web/disk/`.
Filenames must be <= 8 chars (CP/M's 8.3 limit) - that's why it's `DASH`, not
`DASHBOARD`.

## The header

| call | emits | does |
|------|-------|------|
| `clrscr()` | `ESC[2J ESC[H` | clear + home |
| `gotoxy(x,y)` | `ESC[y;xH` | move cursor (1-based, x=col y=row) |
| `clreol()` | `ESC[K` | erase to end of line |
| `textcolor(c)` / `textbackground(c)` | `ESC[3Nm` / `ESC[4Nm` | 8 colours |
| `textbright()` / `textreverse()` / `textreset()` | `ESC[1m` / `ESC[7m` / `ESC[0m` | attributes |
| `cursor_hide()` / `cursor_show()` | `ESC[?25l` / `ESC[?25h` | DECTCEM |
| `waitkey()` | (BDOS read) | one keypress, no echo (alias for z88dk `getkey`) |

## The demos

- **colors.c** - every foreground colour, every background, bright vs normal.
  The one thing `<conio.h>` could never do.
- **dash.c** - a live dashboard: counters and a progress bar rewritten *in place*
  with `gotoxy`, no scrolling. This is the whole point of cursor addressing and
  the basis of every CP/M full-screen tool.
- **menu.c** - an interactive menu: arrow keys (or `j`/`k`) move a reverse-video
  highlight bar, RETURN selects, `Q` quits. The RunCPM terminal maps the arrow
  keys to `h`/`j`/`k`/`l`, so the same code reads arrows and vi keys.

## Portability note

The only RunCPM-specific assumption is the arrow-key -> `hjkl` mapping in
`menu.c`. On a real VT100 the arrows send `ESC [ A/B/C/D`; the menu reads the
final letter too, so it works both ways.
