/* CP/M 2.2 welcome banner, run once at cold boot via AUTOEXEC.TXT.

   RunCPM is built with BOOTONLY=TRUE, so the autoexec command fires exactly
   once (firstBoot clears after), not on every warm boot. That's why this can
   stay a plain print with no run-once guard of its own. */
#include <stdio.h>

int main(void) {
    /* VT100 clear + home. z88dk's clrscr() emits a code RunCPM's terminal
       doesn't map, so drive the terminal directly (ESC[2J clears, ESC[H homes). */
    printf("\x1b[2J\x1b[H");
    
    
    printf("STARTING CP/M 2.2 ...\r\n\r\n");

    printf( "   ____ ____   ____  __ \r\n");
    printf( "  / ___|  _ \\ / /  \\/  |\r\n");
    printf( " | |   | |_) / /| |\\/| |\r\n");
    printf( " | |___|  __/ / | |  | |\r\n");
    printf( "  \\____|_| /_/  |_|  |_|\r\n");
    printf( "                        \r\n\r\n");

    return 0;
}
