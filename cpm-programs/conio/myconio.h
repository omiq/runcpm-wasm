/* myconio.h - portable ANSI/VT100 console for CP/M C (z88dk +cpm, any clib).
 *
 * Why this exists
 * ---------------
 * z88dk's <conio.h> is hard-wired to ADM-3A escape codes (ESC= for cursor,
 * 0x1A for clear) and has NO colour. Prove it:
 *     zcc +cpm --generic-console foo.c    ->  gotoxy emits  ESC = <row+32><col+32>
 * That only renders on a 1976 Lear-Siegler terminal. Real RC2014 CP/M and the
 * RunCPM browser terminal both speak ANSI/VT100, so we emit ANSI ourselves. One
 * binary then renders identically on RunCPM (xterm.js), a real RC2014 over
 * serial, and any ANSI terminal - and we get colour, which conio never had.
 *
 * Coordinates are 1-based. ANSI orders them row;col; gotoxy(x,y) here takes
 * x,y (column,row) to match conio habit and flips them internally.
 *
 * No dependency beyond <stdio.h>. Build: zcc +cpm -create-app foo.c -o foo
 */
#ifndef MYCONIO_H
#define MYCONIO_H
#include <stdio.h>

/* ---- screen + cursor ------------------------------------------------ */
#define clrscr()          fputs("\x1b[2J\x1b[H", stdout)
#define gotoxy(x, y)      printf("\x1b[%d;%dH", (int)(y), (int)(x))
#define clreol()          fputs("\x1b[K", stdout)          /* erase to end of line */
#define cursor_home()     fputs("\x1b[H", stdout)
#define cursor_hide()     fputs("\x1b[?25l", stdout)
#define cursor_show()     fputs("\x1b[?25h", stdout)

/* ---- colour (SGR) --------------------------------------------------- */
/* 0=black 1=red 2=green 3=yellow 4=blue 5=magenta 6=cyan 7=white */
#define COL_BLACK   0
#define COL_RED     1
#define COL_GREEN   2
#define COL_YELLOW  3
#define COL_BLUE    4
#define COL_MAGENTA 5
#define COL_CYAN    6
#define COL_WHITE   7

#define textcolor(c)      printf("\x1b[%dm", 30 + (int)(c))
#define textbackground(c) printf("\x1b[%dm", 40 + (int)(c))
#define textbright()      fputs("\x1b[1m",  stdout)   /* bold / bright */
#define textdim()         fputs("\x1b[2m",  stdout)
#define textreverse()     fputs("\x1b[7m",  stdout)   /* swap fg/bg - great for menu bars */
#define textreset()       fputs("\x1b[0m",  stdout)   /* back to defaults */

/* ---- character output ----------------------------------------------- */
#define cputs(s)          fputs((s), stdout)
#define cputch(ch)        putchar((int)(ch))

/* ---- keyboard ------------------------------------------------------- */
/* Single keypress, no line buffering. z88dk's <stdio.h> already provides
   getkey() = fgetc_cons() (a raw BDOS console read returning one key), so we
   reuse it and expose a readable alias. RunCPM and RC2014 both return per
   keystroke, not per line. The RunCPM browser maps the arrow keys to the bytes
   h/j/k/l, so a portable "up/down" menu can read 'k'/'j' as well as raw VT100
   arrows. */
#define waitkey()         getkey()

#endif
