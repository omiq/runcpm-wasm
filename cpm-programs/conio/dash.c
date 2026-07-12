/* dashboard.c - why conio matters: rewrite fields IN PLACE, no scrolling.
   Plain printf can only append and scroll. With gotoxy we park the cursor on
   the same cells each tick and overwrite them, so counters and a bar animate
   in a fixed layout - the foundation of every CP/M full-screen tool. */
#include "myconio.h"

/* crude delay: a volatile busy-loop (z88dk -O2 would optimise a plain loop
   away). RunCPM yields to the browser periodically so the tab stays alive. */
static void delay(void) {
    volatile unsigned i;
    for (i = 0; i < 12000; i++) { /* spin */ }
}

static void draw_frame(void) {
    int x;
    clrscr();
    textbright(); textcolor(COL_CYAN);
    gotoxy(20, 1); cputs("CP/M live dashboard  (myconio.h)");
    textreset();
    /* a simple box using ASCII, drawn once */
    gotoxy(4, 3);  cputs("+----------------------------------------------------------+");
    for (x = 4; x <= 16; x++) { gotoxy(4, x); cputch('|'); gotoxy(63, x); cputch('|'); }
    gotoxy(4, 16); cputs("+----------------------------------------------------------+");

    gotoxy(8, 5);  textcolor(COL_YELLOW); cputs("ticks   :"); textreset();
    gotoxy(8, 7);  textcolor(COL_YELLOW); cputs("squared :"); textreset();
    gotoxy(8, 9);  textcolor(COL_YELLOW); cputs("progress:"); textreset();
    gotoxy(8, 13); textcolor(COL_YELLOW); cputs("status  :"); textreset();
}

int main(void) {
    int t, x;
    const int TICKS = 40;

    cursor_hide();
    draw_frame();

    for (t = 0; t <= TICKS; t++) {
        /* counters: overwrite the same cells every tick */
        gotoxy(20, 5);  textbright(); textcolor(COL_WHITE);
        printf("%4d", t); textreset();

        gotoxy(20, 7);  textcolor(COL_GREEN);
        printf("%6d", t * t); textreset();

        /* progress bar: fill proportion of a 40-wide gutter */
        gotoxy(20, 9); textcolor(COL_GREEN);
        cputch('[');
        for (x = 0; x < 40; x++) {
            if (x < (t * 40) / TICKS) { textbackground(COL_GREEN); cputch(' '); textreset(); textcolor(COL_GREEN); }
            else cputch('.');
        }
        cputch(']');
        printf(" %3d%%", (t * 100) / TICKS);
        textreset();

        /* status flips colour near the end */
        gotoxy(20, 13);
        if (t < TICKS) { textcolor(COL_CYAN); cputs("running..."); }
        else           { textbright(); textcolor(COL_GREEN); cputs("complete! "); }
        textreset();

        delay();
    }

    cursor_show();
    gotoxy(4, 18);
    cputs("done - press RETURN");
    waitkey();
    clrscr();
    return 0;
}
