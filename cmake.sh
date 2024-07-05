#!/bin/bash
set -e
set -x

BIN=$(cat name)

WLP="./src/platform/wayland/"
WLPD="$WLP/wayland-protocols"

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

_clean()
{
    rm -rf build $WLPD
}

release()
{
    _clean
    wayland

    if cmake -GNinja -S . -B build/ -DCMAKE_BUILD_TYPE=Release "$@"
    then
        cmake --build build/ -j -v
    fi
}

default()
{
    _clean
    wayland

    if cmake -GNinja -S . -B build/ -DCMAKE_BUILD_TYPE=RelWithDebInfo "$@"
    then
        cmake --build build/ -j -v
    fi
}

debug()
{
    _clean
    wayland

    if CC=clang CXX=clang++ CC_LD=mold CXX_LD=mold cmake -G "Ninja" -S . -B build/ -DCMAKE_BUILD_TYPE=Debug "$@"
    then
        cmake --build build/ -j -v
    fi
}

asan()
{
    _clean
    wayland

    if CC=clang CXX=clang++ CC_LD=mold CXX_LD=mold cmake -G "Ninja" -S . -B build/ -DCMAKE_BUILD_TYPE=Asan "$@"
    then
        cmake --build build/ -j -v
    fi
}

build()
{
    cmake --build build/ -j -v
}

run()
{
    if cmake --build build/ -j -v
    then
        echo ""
        # ASAN_OPTIONS=detect_leaks=1 LSAN_OPTIONS=suppressions=leaks.txt ./build/$BIN "$@" # 2> /tmp/$BIN-dbg.txt
        # ASAN_OPTIONS=halt_on_error=0 ./build/$BIN "$@" # 2> /tmp/$BIN-dbg.txt
        ./build/$BIN "$@" 2> /tmp/$BIN-dbg.txt
    fi
}

_install()
{
    cmake --install build/
}

_uninstall()
{
    sudo xargs rm < ./build/install_manifest.txt
}

_test()
{
    ./tests/test.sh
}

cd $(dirname $0)

case "$1" in
    default) default "${@:2}" ;;
    run) run "${@:2}" ;;
    debug) debug "${@:2}" ;;
    asan) asan "${@:2}" ;;
    release) release "${@:2}";;
    install) _install ;;
    uninstall) _uninstall ;;
    clean) _clean ;;
    test) _test ;;
    *) build ;;
esac
