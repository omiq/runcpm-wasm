#!/usr/bin/env bash
# Rebuild the demo .COM files from source and stage them onto the default disk.
#   HELLO.COM  <- hello.c   via z88dk's +cpm target (needs z88dk on PATH)
#   GREET.COM  <- greet.asm via pasmo
# Both are MIT, part of runcpm-wasm. Run from this directory.
set -euo pipefail
cd "$(dirname "$0")"

: "${ZCC:=zcc}"        # override if z88dk lives elsewhere, e.g. ZCC=~/z88dk/bin/zcc
"$ZCC" +cpm -create-app hello.c -o hello   # emits HELLO.COM
pasmo greet.asm GREET.COM

cp HELLO.COM GREET.COM README.TXT ../../web/disk/
echo "built HELLO.COM ($(wc -c <HELLO.COM) b), GREET.COM ($(wc -c <GREET.COM) b); staged to web/disk/"
