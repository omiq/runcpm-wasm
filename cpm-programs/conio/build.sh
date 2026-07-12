#!/usr/bin/env bash
# Build the myconio.h demo .COM files with z88dk's +cpm target and stage them
# onto the browser disk (web/disk/). Each demo is plain ANSI via myconio.h - no
# <conio.h>, so one binary runs on RunCPM, a real RC2014, or any ANSI terminal.
# Run from this directory. Override the compiler with ZCC=~/z88dk/bin/zcc.
set -euo pipefail
cd "$(dirname "$0")"
: "${ZCC:=zcc}"

for src in colors dash menu; do
  "$ZCC" +cpm -create-app "$src.c" -o "$src"      # emits <SRC>.COM (uppercased)
  com="$(printf '%s' "$src" | tr '[:lower:]' '[:upper:]').COM"
  echo "built $com ($(wc -c < "$com") b)"
done

cp COLORS.COM DASH.COM MENU.COM ../../web/disk/
echo "staged COLORS.COM DASH.COM MENU.COM -> web/disk/"
