#!/usr/bin/env bash
# Extract files from a MYZ80 CP/M disk image (A.DSK / B.DSK / C.DSK, as found
# inside the old js-dos "CPM.zip") into loose host files, using cpmtools.
#
# MYZ80's on-disk layout is non-standard and does NOT match any stock cpmtools
# diskdef: the directory sits in a 256-byte reserved zone and the data area
# uses 4K allocation blocks. The geometry below was solved by locating a known
# file (rogue-vt.com) inside A.DSK and back-computing the block size and offset
# until the extracted bytes matched the original md5 exactly:
#
#     seclen 128   sectrk 128   blocksize 4096   boottrk 0   offset 256   maxdir 512
#
# MYZ80 uses 16-bit block pointers on ALL its disks. cpmtools, though, picks
# 8-bit pointers when a disk has <=255 blocks (small disks: B.DSK, C.DSK), which
# misreads every block after the first and corrupts multi-block files (MBASIC,
# the Hi-Tech C compiler). So we force 16-bit by padding the image (with the CP/M
# empty-sector byte 0xE5) up past 256 blocks and sizing 'tracks' to match. The
# padding is unused space; files only reference the real low blocks.
#
# Usage: ./extract-myz80.sh A.DSK outdir/
set -euo pipefail
DSK="${1:?usage: extract-myz80.sh <image.dsk> <outdir>}"
OUT="${2:?usage: extract-myz80.sh <image.dsk> <outdir>}"

# Pad a working copy to at least 260 blocks * 4096 so cpmtools uses 16-bit pointers.
WORK="$(mktemp).dsk"
MIN=$(( 260 * 4096 + 256 ))
python3 - "$DSK" "$WORK" "$MIN" <<'PY'
import sys
src, dst, minsz = sys.argv[1], sys.argv[2], int(sys.argv[3])
d = bytearray(open(src,'rb').read())
if len(d) < minsz: d += b'\xe5' * (minsz - len(d))
open(dst,'wb').write(d)
PY
DSK="$WORK"
SIZE=$(wc -c < "$DSK")
TRACKS=$(( (SIZE - 256 + 16383) / 16384 ))

DISKDEFS="$(brew --prefix 2>/dev/null)/share/diskdefs"
[ -f "$DISKDEFS" ] || DISKDEFS=/etc/cpmtools/diskdefs

# Append (or refresh) a "myz80x" definition sized for this image.
python3 - "$DISKDEFS" "$TRACKS" <<'PY'
import re, sys
path, tracks = sys.argv[1], sys.argv[2]
s = open(path).read()
s = re.sub(r'\ndiskdef myz80x\n.*?\nend\n', '\n', s, flags=re.S)
s += (f"diskdef myz80x\n  seclen 128\n  tracks {tracks}\n  sectrk 128\n"
      f"  blocksize 4096\n  maxdir 512\n  boottrk 0\n  offset 256\n  os 2.2\nend\n")
open(path, 'w').write(s)
PY

mkdir -p "$OUT"
cpmls -f myz80x "$DSK" || true
cpmcp -f myz80x "$DSK" '0:*.*' "$OUT/"
echo "extracted to $OUT ($(ls "$OUT" | wc -l | tr -d ' ') files)"
