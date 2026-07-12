/* Same marker-bracketed probe, but via myconio.h (ANSI). Compare bytes to the
   ADM-3A run: these should be clean ESC[ sequences. */
#include "myconio.h"

int main(void) {
    putchar('|'); clrscr();
    putchar('|'); gotoxy(10, 5);
    putchar('|'); textcolor(4);
    putchar('|'); textbackground(1);
    putchar('|'); cputs("HI");
    putchar('|'); gotoxy(1, 20);
    putchar('|'); textreset();
    return 0;
}
