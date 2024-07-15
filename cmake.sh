#!/bin/bash
set -e
set -x

BIN=$(cat name)

_clean()
{
    rm -rf build
}

release()
{
    _clean

    if cmake -GNinja -S . -B build/ -DCMAKE_BUILD_TYPE=Release "$@"
    then
        cmake --build build/ -j -v
    fi
}

default()
{
    _clean

    if cmake -GNinja -S . -B build/ -DCMAKE_BUILD_TYPE=RelWithDebInfo "$@"
    then
        cmake --build build/ -j -v
    fi
}

debug()
{
    _clean

    if CC=clang CXX=clang++ CC_LD=mold CXX_LD=mold cmake -G "Ninja" -S . -B build/ -DCMAKE_BUILD_TYPE=Debug "$@"
    then
        cmake --build build/ -j -v
    fi
}

asan()
{
    _clean

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
