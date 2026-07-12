/* myconio.h - portable ANSI/VT100 console for CP/M C (z88dk +cpm, any clib).
 *
 * Why this exists: z88dk's <conio.h> is hard-wired to ADM-3A escape codes
 * (ESC= cursor, 0x1A clear) and has no colour. Real RC2014 CP/M and RunCPM
 * both drive ANSI/VT100 terminals, so we emit ANSI directly. One binary then
 * renders identically on RunCPM (xterm.js), a real RC2014 over serial, and any
 * ANSI terminal. Coordinates are 1-based; note ANSI order is row;col (y;x),
 * the reverse of conio's gotoxy(x,y) - kept as (x,y) here to match habit.
 */
#ifndef MYCONIO_H
#define MYCONIO_H
#include <stdio.h>

#define clrscr()        fputs("\x1b[2J\x1b[H", stdout)
#define gotoxy(x, y)    printf("\x1b[%d;%dH", (int)(y), (int)(x))
#define clreol()        fputs("\x1b[K", stdout)
#define cursor_home()   fputs("\x1b[H", stdout)
#define cursor_hide()   fputs("\x1b[?25l", stdout)
#define cursor_show()   fputs("\x1b[?25h", stdout)

/* colours: 0=black 1=red 2=green 3=yellow 4=blue 5=magenta 6=cyan 7=white */
#define textcolor(c)    printf("\x1b[%dm", 30 + (int)(c))
#define textbackground(c) printf("\x1b[%dm", 40 + (int)(c))
#define textbright()    fputs("\x1b[1m", stdout)
#define textreset()     fputs("\x1b[0m", stdout)

#define cputs(s)        fputs((s), stdout)
#define cputch(ch)      putchar((int)(ch))

#endif
