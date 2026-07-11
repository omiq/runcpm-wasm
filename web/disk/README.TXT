RUNCPM-WASM  -  CP/M 2.2 IN A BROWSER TAB
=========================================

This is a default disk with a few small programs on it. All of them are
MIT licensed and built from source in this project. No Digital Research
utilities (PIP, STAT, ED, ...) are bundled, because those are still under
copyright; add your own copy if you want them.

WHAT IS HERE
  HELLO   COM   Hello world, written in C and compiled with z88dk +cpm.
  GREET   COM   The same idea in Z80 assembly (BDOS function 9).
  ROGUE   COM   A 1985 CP/M port of Rogue (see the project README for
                provenance). Type ROGUE to play. Best in the VT100 page.
  README  TXT   This file. Type: TYPE README.TXT

BUILT-IN COMMANDS (these are in the CCP, not files on the disk)
  DIR           List the files on the current drive.
  TYPE fn       Show a text file.
  ERA fn        Erase a file.
  REN new=old   Rename a file.
  USER n        Switch user area (0-15).

ADDING YOUR OWN PROGRAMS
  Write a .COM into /A/0 from the page before boot (see web/rogue.html),
  or compile C to a .COM with z88dk's +cpm target and drop it in.

Have fun.
