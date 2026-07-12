#include "myconio.h"

int main(void) {
    int c;
    clrscr();
    cursor_hide();
    gotoxy(20, 2);  textbright(); textcolor(3);
    cputs("myconio.h - ANSI colour on RunCPM");
    textreset();
    for (c = 0; c < 8; c++) {
        gotoxy(10, 5 + c);
        textcolor(c);
        printf("colour %d: ", c);
        textbackground(c); textcolor(7);
        cputs("  BLOCK  ");
        textreset();
    }
    gotoxy(10, 15); textbright(); textcolor(2);
    cputs("gotoxy + colour both working. Press RETURN.");
    textreset();
    cursor_show();
    gotoxy(1, 17);
    getchar();
    clrscr();
    return 0;
}
