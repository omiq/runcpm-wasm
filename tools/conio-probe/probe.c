/* conio dialect probe: exercise the escape-emitting conio calls so we can
   capture the raw byte stream z88dk's +cpm conio produces. Marker bytes ('|')
   bracket each call to make the hexdump readable. */
#include <conio.h>
#include <stdio.h>

int main(void) {
    putch('|'); clrscr();
    putch('|'); gotoxy(10, 5);
    putch('|'); textcolor(4);
    putch('|'); textbackground(1);
    putch('|'); cputs("HI");
    putch('|'); gotoxy(1, 20);
    putch('|');
    return 0;
}
