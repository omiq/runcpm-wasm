/* menu.c - an interactive TUI menu: move with the arrow keys (or j/k),
   RETURN to pick, Q to quit. Uses reverse-video (ESC[7m) for the highlight
   bar and gotoxy to redraw only what changed. This is the pattern behind
   WordStar-era CP/M menus.

   Key handling note: the RunCPM browser terminal maps the arrow keys to the
   bytes h/j/k/l, so this reads BOTH raw arrows-as-hjkl and literal j/k. On a
   real VT100/RC2014 the arrow keys send ESC [ A/B; we read the final letter. */
#include "myconio.h"

static const char *ITEM[] = {
    "New file",
    "Open file",
    "Save",
    "Settings",
    "About",
    "Quit",
};
#define N (int)(sizeof(ITEM)/sizeof(ITEM[0]))

static void draw(int sel) {
    int i;
    gotoxy(6, 2); textbright(); textcolor(COL_CYAN);
    cputs("myconio.h menu  -  up/down or j/k, RETURN selects, Q quits");
    textreset();

    for (i = 0; i < N; i++) {
        gotoxy(10, 4 + i);
        if (i == sel) { textreverse(); printf("  %-20s", ITEM[i]); textreset(); }
        else          {               printf("  %-20s", ITEM[i]);              }
    }
}

int main(void) {
    int sel = 0, ch;

    clrscr();
    cursor_hide();
    draw(sel);

    for (;;) {
        ch = waitkey();

        if (ch == 'k' || ch == 'A')      { sel = (sel + N - 1) % N; }  /* up   (k / arrow) */
        else if (ch == 'j' || ch == 'B') { sel = (sel + 1) % N; }      /* down (j / arrow) */
        else if (ch == '\r' || ch == '\n') {                           /* select */
            if (sel == N - 1) break;                                   /* Quit item */
            gotoxy(10, 4 + N + 2); clreol();
            textbright(); textcolor(COL_GREEN);
            printf("selected: %s", ITEM[sel]); textreset();
        }
        else if (ch == 'q' || ch == 'Q') break;
        else if (ch == 0x1b) continue;   /* swallow the ESC of an ESC[A sequence */

        draw(sel);
    }

    cursor_show();
    clrscr();
    cputs("bye\r\n");
    return 0;
}
