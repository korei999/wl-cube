#!/bin/sh
set -e
set -x

BIN=$(cat name)

cd $(dirname $0)
if make debug
then
    echo ""
    # ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=leaks.txt ./build/$BIN "$@" # 2> /tmp/$BIN-dbg.txt
    ./build/$BIN "$@" # 2> /tmp/$BIN-dbg.txt
fi
