/* Hello world for CP/M 2.2, built with z88dk's +cpm target.
   Demonstrates the compile-your-own-.COM path: this .c is turned into
   HELLO.COM and dropped onto drive A. MIT licensed, part of runcpm-wasm. */
#include <stdio.h>

int main(void) {
    printf("Hello from CP/M, compiled to a .COM with z88dk.\r\n");
    printf("Type DIR to see the disk, or ROGUE to play.\r\n");
    return 0;
}
