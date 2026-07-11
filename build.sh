#!/usr/bin/env bash
# Build RunCPM to WebAssembly for in-browser CP/M 2.2.
#
# The port lives entirely outside upstream RunCPM (which is a git submodule):
#   src/abstraction_emscripten.h  copied into the submodule before compiling.
#   src/emscripten.patch          two small edits to upstream main.c + cpu1.h:
#                                   - main.c  selects our abstraction under
#                                     __EMSCRIPTEN__
#                                   - cpu1.h  yields to the browser event loop
#                                     every N instructions so a long guest loop
#                                     (level generation, delay loops) can't
#                                     freeze the tab
#
# Console I/O is bridged to JS (globalThis.CPM) and the blocking _getch yields
# to the browser via emscripten_sleep, which is why -sASYNCIFY is required.
#
# CCP_INTERNAL (RunCPM's default CCP) means no Digital Research code ships:
# RunCPM reimplements the BDOS/BIOS and provides its own MIT command processor.
# Drives live under the MEMFS working directory as A/0/... (FILEBASE "./").
set -euo pipefail
cd "$(dirname "$0")"

if [ ! -f RunCPM/RunCPM/main.c ]; then
  echo "RunCPM submodule missing. Run: git submodule update --init" >&2
  exit 1
fi

# Apply our upstream edits idempotently: if the patch already reverses cleanly
# it is applied, so skip; otherwise apply it.
if git -C RunCPM apply --reverse --check ../src/emscripten.patch 2>/dev/null; then
  echo "patch already applied"
else
  git -C RunCPM apply ../src/emscripten.patch
  echo "patch applied"
fi

cp src/abstraction_emscripten.h RunCPM/RunCPM/abstraction_emscripten.h

: "${EMSDK:=$HOME/emsdk}"
# shellcheck disable=SC1091
source "$EMSDK/emsdk_env.sh" >/dev/null 2>&1 || true

mkdir -p web
emcc RunCPM/RunCPM/main.c \
  -O2 \
  -DCCP_INTERNAL \
  -I RunCPM/RunCPM \
  -sASYNCIFY \
  -sASYNCIFY_STACK_SIZE=81920 \
  -sALLOW_MEMORY_GROWTH \
  -sFORCE_FILESYSTEM \
  -sMODULARIZE \
  -sEXPORT_ES6 \
  -sEXPORT_NAME=createRunCPM \
  -sINVOKE_RUN=0 \
  -sEXPORTED_RUNTIME_METHODS=callMain,FS,ccall,cwrap \
  -o web/runcpm.js

echo "built web/runcpm.js ($(du -h web/runcpm.wasm | cut -f1) wasm)"
