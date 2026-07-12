/* colors.c - the thing z88dk's <conio.h> could never do: ANSI colour on CP/M.
   Shows all 8 foreground colours, all 8 backgrounds, and bright vs normal. */
#include "myconio.h"

static const char *NAME[8] = {
    "black","red","green","yellow","blue","magenta","cyan","white"
};

int main(void) {
    int c;

    clrscr();
    cursor_hide();

    gotoxy(24, 1); textbright(); textcolor(COL_CYAN);
    cputs("myconio.h  -  ANSI colour on CP/M"); textreset();

    gotoxy(4, 3); textdim(); cputs("foreground"); textreset();
    for (c = 0; c < 8; c++) {
        gotoxy(4, 4 + c);
        textcolor(c);
        printf("%-8s the quick brown fox", NAME[c]);
        textreset();
    }

    gotoxy(44, 3); textdim(); cputs("background"); textreset();
    for (c = 0; c < 8; c++) {
        gotoxy(44, 4 + c);
        textbackground(c);
        textcolor(c == COL_WHITE ? COL_BLACK : COL_WHITE);
        printf(" %-8s BLOCK ", NAME[c]);
        textreset();
    }

    gotoxy(4, 14); textbright(); textcolor(COL_GREEN);
    cputs("bright: ");
    for (c = 1; c < 8; c++) { textcolor(c); cputs("##"); }
    textreset();
    gotoxy(4, 15); textcolor(COL_GREEN);
    cputs("normal: "); textdim();
    for (c = 1; c < 8; c++) { textcolor(c); cputs("##"); }
    textreset();

    cursor_show();
    gotoxy(4, 18);
    cputs("press RETURN to exit");
    waitkey();
    clrscr();
    return 0;
}
