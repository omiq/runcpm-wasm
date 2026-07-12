#!/usr/bin/env bash
# conio dialect probe rig.
# Builds probe.c with a given set of extra zcc flags, runs the resulting .COM
# under NATIVE RunCPM (byte-clean putchar), and hexdumps the marker-bracketed
# region so we can see exactly which escape bytes each conio call emits.
#
# probe.c brackets every conio call with putch('|') (0x7c):
#   | clrscr | gotoxy(10,5) | textcolor(4) | textbackground(1) | cputs("HI") | gotoxy(1,20) |
#
# Usage: ./run-probe.sh "<label>" "<extra zcc flags>"
set -euo pipefail
cd "$(dirname "$0")"
export PATH="$HOME/z88dk/bin:$PATH"
export ZCCCFG="$HOME/z88dk/lib/config"

LABEL="$1"; FLAGS="${2:-}"
RUNCPM="$(cd ../.. && pwd)/RunCPM/RunCPM/RunCPM"
WORK="$(mktemp -d)"
mkdir -p "$WORK/A/0"

# shellcheck disable=SC2086
if ! zcc +cpm -create-app $FLAGS -o "$WORK/probe" probe.c > "$WORK/build.log" 2>&1; then
  echo "[$LABEL] BUILD FAILED:"; tail -6 "$WORK/build.log"; echo; exit 0
fi
cp "$WORK/PROBE.COM" "$WORK/A/0/PROBE.COM"

( printf 'PROBE\r'; sleep 1 ) | ( cd "$WORK" && "$RUNCPM" ) > "$WORK/out.bin" 2>&1 &
PID=$!; sleep 1.8; kill $PID 2>/dev/null || true; wait 2>/dev/null || true

# isolate the region after "A0>PROBE\r\r\n" up to the next "\r\n\r\n"
echo "===== [$LABEL]  flags: ${FLAGS:-<none>} ====="
python3 - "$WORK/out.bin" <<'PY'
import sys,re
d=open(sys.argv[1],'rb').read()
m=re.search(rb'A0>PROBE\r\r\n(.*?)\r\n\r\nRunCPM',d,re.S)
seg=m.group(1) if m else d
out=[]
for b in seg:
    if b==0x7c: out.append('  | ')
    elif b==0x1b: out.append(' ESC')
    elif 32<=b<127: out.append('  '+chr(b))
    else: out.append(' %02x'%b)
print('bytes:',' '.join('%02x'%b for b in seg))
print('read :',''.join(out))
PY
