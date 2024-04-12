#!/bin/sh

BIN=$(cat name)

cd $(dirname $0)
./build/$BIN "$@"
