#!/bin/sh
set -e

cd $(dirname $0)

WLP="./platform/wayland/"
WLPD="$WLP/wayland-protocols"

set -x

wayland()
{
    WAYLAND_PROTOCOLS_DIR=$(pkg-config wayland-protocols --variable=pkgdatadir)
    WAYLAND_SCANNER=$(pkg-config --variable=wayland_scanner wayland-scanner)
    XDG_SHELL="$WAYLAND_PROTOCOLS_DIR/stable/xdg-shell/xdg-shell.xml"
    POINTER_CONSTRAINTS="$WAYLAND_PROTOCOLS_DIR/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml"
    RELATIVE_POINTER="$WAYLAND_PROTOCOLS_DIR/unstable/relative-pointer/relative-pointer-unstable-v1.xml"

    mkdir -p $WLPD

    $WAYLAND_SCANNER client-header $RELATIVE_POINTER $WLPD/relative-pointer-unstable-v1.h
    $WAYLAND_SCANNER private-code $RELATIVE_POINTER $WLPD/relative-pointer-unstable-v1.c
    $WAYLAND_SCANNER client-header $POINTER_CONSTRAINTS $WLPD/pointer-constraints-unstable-v1.h
    $WAYLAND_SCANNER private-code $POINTER_CONSTRAINTS $WLPD/pointer-constraints-unstable-v1.c
    $WAYLAND_SCANNER client-header $XDG_SHELL $WLPD/xdg-shell.h
    $WAYLAND_SCANNER private-code $XDG_SHELL $WLPD/xdg-shell.c
}

clean()
{
    rm -rf build $WLPD
}

debug()
{
    clean
    wayland

    if meson setup build -Db_sanitize=address,undefined --buildtype=debug
    then
        ninja -C build/
    fi
}

release()
{
    clean
    wayland

    if meson setup build
    then
        ninja -C build/
    fi
}

run()
{
    BIN=wl-cube

    if ninja -C build/
    then
        echo ""
        # ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=leaks.txt ./build/$BIN "$@" # 2> /tmp/$BIN-dbg.txt
        # ASAN_OPTIONS=halt_on_error=0 ./build/$BIN "$@" # 2> /tmp/$BIN-dbg.txt
        # ./build/$BIN "$@" # 2> /tmp/$BIN-dbg.txt
        ./build/$BIN "$@" 2> /tmp/$BIN-dbg.txt
    fi
}

case "$1" in
    debug) debug ;;
    run) run "$@" ;;
    clean) clean ;;
    *) release ;;
esac
